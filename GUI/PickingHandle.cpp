// PickingHandle.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "PickingHandle.h"
#include "COGLPickingView.h"
#include  <math.h>
//FEM
#include "elements.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include ".\pickinghandle.h"

// PickingHandle dialog

IMPLEMENT_DYNAMIC(PickingHandle, CDialog)
PickingHandle::PickingHandle(CWnd* pParent /*=NULL*/)
	: CDialog(PickingHandle::IDD, pParent)
{
}

PickingHandle::~PickingHandle()
{
}

void PickingHandle::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PickingHandle, CDialog)
    ON_BN_CLICKED(IDC_SELECT_MODE, OnBnClickedSelectMode)
    ON_BN_CLICKED(IDC_DESELECT_MODE, OnBnClickedDeselectMode)
    ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
    ON_BN_CLICKED(IDC_DESELECTALL, OnBnClickedDeselectall)
    ON_BN_CLICKED(IDC_STOP_PICKING, OnBnClickedStopPicking)
    ON_BN_CLICKED(IDC_SELECT_PLANE, OnBnClickedSelectPlane)
END_MESSAGE_MAP()


// PickingHandle message handlers

void PickingHandle::OnBnClickedSelectMode()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	theApp->mEditMode = TRUE;
	theApp->mMouseRotateMode = FALSE;

	if(theApp->mEditMode == TRUE)
	{
		theApp->mSelectMode = TRUE;
		theApp->mDeselectMode = FALSE;
		theApp->mMouseRotateMode = FALSE;	

		// Disable the other
		CheckDlgButton(IDC_DESELECT_MODE, 0);
	}

	theApp->mContinuous = TRUE;

	// Enable Stop button
	GetDlgItem(IDC_STOP_PICKING)->EnableWindow();
}

void PickingHandle::OnBnClickedDeselectMode()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    theApp->mEditMode = TRUE;
	theApp->mMouseRotateMode = FALSE;

	if(theApp->mEditMode == TRUE)
	{
		theApp->mDeselectMode = TRUE;
		theApp->mSelectMode = FALSE;
		theApp->mMouseRotateMode = FALSE;	

		// Uncheck the other
		CheckDlgButton(IDC_SELECT_MODE, 0);
	}

	theApp->mContinuous = TRUE;
	// Enable Stop button
	GetDlgItem(IDC_STOP_PICKING)->EnableWindow();
}

void PickingHandle::OnBnClickedSelectall()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

	UpdateData(TRUE);
	if(theApp->ElementSwitch == 1)
	{
		theApp->hitsElementTotal = 0;
		for(int i=0; i< ElListSize(); ++i)
		{
			++theApp->hitsElementTotal;
			theApp->elementPickedTotal = (int *)realloc(theApp->elementPickedTotal, theApp->hitsElementTotal*sizeof(int));
			theApp->elementPickedTotal[theApp->hitsElementTotal-1] = i;
		}
	}
	if(theApp->PolylineSwitch == 1)
	{
		theApp->hitsPolylineTotal = 0;

        // Getting the number of polylines
        int numberOfPolylines = (int)polyline_vector.size();

        for(int i=0; i< numberOfPolylines; ++i)
		{
			++theApp->hitsPolylineTotal;
			theApp->polylinePickedTotal = (int *)realloc(theApp->polylinePickedTotal, theApp->hitsPolylineTotal*sizeof(int));
			theApp->polylinePickedTotal[theApp->hitsPolylineTotal-1] = i;
		}
	}
	if(theApp->GLINodeSwitch == 1)
	{
		theApp->hitsGLINodeTotal = 0;

		// get the number of gli points from COGLPickingView
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		
		int numberOfGLIPoints = pView->numberOfGLIPoints();

		for(int i=0; i < numberOfGLIPoints; ++i)
		/* The process to get rid of the .rfi file
		for(int i=0; i < NodeListSize (); ++i)
		*/
		{
			++theApp->hitsGLINodeTotal;
			theApp->GLInodePickedTotal = (int *)realloc(theApp->GLInodePickedTotal, theApp->hitsGLINodeTotal*sizeof(int));
			theApp->GLInodePickedTotal[theApp->hitsGLINodeTotal-1] = i;
		}
	}
	if(theApp->RFINodeSwitch == 1)
	{
		theApp->hitsRFINodeTotal = 0;

		// get the number of gli points from COGLPickingView
		// Let's open the door to COGLPickingView
		// Update the change by redrawing
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
		// Get the active view attached to the active MDI child window.
		COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
		pView=pView;//TK

		for(int i=0; i < NodeListSize (); ++i)
		{
			++theApp->hitsRFINodeTotal;
			theApp->RFInodePickedTotal = (int *)realloc(theApp->RFInodePickedTotal, theApp->hitsRFINodeTotal*sizeof(int));
			theApp->RFInodePickedTotal[theApp->hitsRFINodeTotal-1] = i;
		}
	}
    if(theApp->SurfaceSwitch == 1)
	{
		theApp->hitsSurfaceTotal = 0;

        // Getting the number of polylines
        int numberOfSurfaces =  (int)surface_vector.size();//CC

		for(int i=0; i< numberOfSurfaces; ++i)
		{
			++theApp->hitsSurfaceTotal;
			theApp->surfacePickedTotal = (int *)realloc(theApp->surfacePickedTotal, theApp->hitsSurfaceTotal*sizeof(int));
			theApp->surfacePickedTotal[theApp->hitsSurfaceTotal-1] = i;
		}
	}

	// Do some real time selection in the list
	if(theApp->pPolyline->GetSafeHwnd() != NULL)
	{
		theApp->pPolyline->OnUpdatePointListPicked();
		theApp->pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp->pSurface->GetSafeHwnd() != NULL)
	{
		theApp->pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp->pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp->pVolume->GetSafeHwnd() != NULL)
	{
		theApp->pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	showChange();
}

void PickingHandle::OnBnClickedDeselectall()
{
    UpdateData(TRUE);

	if(theApp.RFINodeSwitch == 1)
	{
		theApp.hitsRFINodeTotal = 0;
//		free(theApp.RFInodePickedTotal);
	}
	if(theApp.ElementSwitch == 1)
	{
		theApp.hitsElementTotal = 0;
//		free(theApp.elementPickedTotal);
	}
	if(theApp.GLINodeSwitch == 1)
	{
		theApp.hitsGLINodeTotal = 0;
//		free(theApp.GLInodePickedTotal);
	}
	if(theApp.PolylineSwitch == 1)
	{
		theApp.hitsPolylineTotal = 0;
//		free(theApp.polylinePickedTotal);
	}
    if(theApp.SurfaceSwitch == 1)
	{
		theApp.hitsSurfaceTotal = 0;
//		free(theApp.surfacePickedTotal);
	}
	if(theApp.VolumeSwitch == 1)
	{
		theApp.hitsVolumeTotal = 0;
//		free(theApp.volumePickedTotal);
	}
	

	// Do some real time selection in the list
	if(theApp.pPolyline->GetSafeHwnd() != NULL)
	{
		theApp.pPolyline->OnUpdatePointListPicked();
		theApp.pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp.pSurface->GetSafeHwnd() != NULL)
	{
		theApp.pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp.pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp.pVolume->GetSafeHwnd() != NULL)
	{
		theApp.pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	showChange();
}

void PickingHandle::OnBnClickedStopPicking()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    // Turn off continuous picking
#ifdef _NONEED_
	(theApp->pViewControlDlg)->OnSelchangeModePublic();
#endif
	theApp->mContinuous = FALSE;

	// These handles for next picking
	theApp->mSelectMode = FALSE;
	theApp->mDeselectMode = FALSE;
	
	// Uncheck the selection or deselection
	CheckDlgButton(IDC_SELECT_MODE, 0);
	CheckDlgButton(IDC_DESELECT_MODE, 0);
	// And inactivate stop button
	GetDlgItem(IDC_STOP_PICKING)->EnableWindow(FALSE);

	// Do some real time selection in the list
	if(theApp->pPolyline->GetSafeHwnd() != NULL)
	{
		theApp->pPolyline->OnUpdatePointListPicked();
		theApp->pPolyline->OnLbnSelOnlyThePickedPolylines();
	}
	else
	{
	}
	if(theApp->pSurface->GetSafeHwnd() != NULL)
	{
		theApp->pSurface->OnUpdatePolylineListPicked();	
		// Let's highlight the picked surfaces in this dialog
		theApp->pSurface->OnLbnSelOnlyThePickedSurfaces();
	}
	if(theApp->pVolume->GetSafeHwnd() != NULL)
	{
		theApp->pVolume->OnUpdateSurfaceListPicked();	
	}
	else
	{
	}

	// redraw the view
	Invalidate(TRUE);
}

void PickingHandle::showChange()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
	pView->Invalidate();
}

void PickingHandle::OnBnClickedSelectPlane()
{
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();

    // Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();

    // Three points to solve the plane and four coeeficients of the plan
    CGLPoint point[3];
    double p[4];
    static long *element_nodes;
    double tolerance = 1e-5;
    int count = 0;

    // Assign three points from the element picked.
    // I'm taking the first element always. The warning message box later on will help users.
    element_nodes = ElGetElementNodes(theApp->elementPickedTotal[0]);
    for(int j=0; j<3; ++j)
    {
        point[j].x = GetNodeX (element_nodes[j]);
        point[j].y = GetNodeY (element_nodes[j]);
        point[j].z = GetNodeZ (element_nodes[j]);

        // Convert the coordinates to OpenGL so that the tolerance is secured.
        point[j] = pView->ConvertScaleToOpenGL(point[j]);
    }
 
    // The plane equation should be p[0]x + p[1]y + p[2]z + p[3] = 0
    solvePlane(point, p);

    // Select the elements in the plane
    if(theApp->hitsElementTotal == 1 && theApp->ElementSwitch == 1)
	{
		for(int i=0; i< ElListSize(); ++i)
		{
            // Get element index
            element_nodes = ElGetElementNodes(i);
            int numOfNodeInElement = 4;

            // Get the coordinates of the center of the element
            double centerX=0.0, centerY=0.0, centerZ=0.0;
            for(int j=0; j< numOfNodeInElement; ++j)
            {
                CGLPoint real, gl;
                real.x = GetNodeX (element_nodes[j]); real.y = GetNodeY (element_nodes[j]); real.z = GetNodeZ (element_nodes[j]);
                gl = pView->ConvertScaleToOpenGL(real);
                centerX += gl.x;
                centerY += gl.y;
                centerZ += gl.z;
            }
            centerX /= numOfNodeInElement; centerY /= numOfNodeInElement; centerZ /= numOfNodeInElement;

            double IsOnThePlane = p[0]* centerX + p[1]*centerY + p[2]*centerZ + p[3];
            
            // Let's see if this element is on the plane
			if(fabs(IsOnThePlane) < tolerance)
				++count;

		}

        // Memory allocation for the new selection
		theApp->hitsElementTotal = count;
		theApp->elementPickedTotal = (int *) realloc(theApp->elementPickedTotal, theApp->hitsElementTotal * sizeof(int));

		// Setting up the plane nodes
		count = 0;
		for(int i=0; i< ElListSize(); ++i)
		{
            // Get element index
            element_nodes = ElGetElementNodes(i);
            int numOfNodeInElement = 4;

            // Get the coordinates of the center of the element
            double centerX=0.0, centerY=0.0, centerZ=0.0;
            for(int j=0; j< numOfNodeInElement; ++j)
            {
                CGLPoint real, gl;
                real.x = GetNodeX (element_nodes[j]); real.y = GetNodeY (element_nodes[j]); real.z = GetNodeZ (element_nodes[j]);
                gl = pView->ConvertScaleToOpenGL(real);
                centerX += gl.x;
                centerY += gl.y;
                centerZ += gl.z;
#ifdef PCH
                centerX += GetNodeX (element_nodes[j]);
                centerY += GetNodeY (element_nodes[j]);
                centerZ += GetNodeZ (element_nodes[j]);
#endif
            }
            centerX /= numOfNodeInElement; centerY /= numOfNodeInElement; centerZ /= numOfNodeInElement;

            double IsOnThePlane = p[0]* centerX + p[1]*centerY + p[2]*centerZ + p[3];
            
            // Let's see if this element is on the plane
			if(fabs(IsOnThePlane) < tolerance)
			{
				theApp->elementPickedTotal[count] = i;
				++count;
			}
		}
    }
}

void PickingHandle::solvePlane(CGLPoint* point, double* p)
{
    //double tolerance = 1e-6;
	
    // Solve the plane coefficients
	// Solve the vector orthogonal to the cutting plane
	// Solve four coefficients of the cutting plane
	double a1 = 0.0, b1 = 0.0, c1 = 0.0;
	double a2 = 0.0, b2 = 0.0, c2 = 0.0;

    a1 = point[0].x - point[2].x;
	b1 = point[0].y - point[2].y;
	c1 = point[0].z - point[2].z;
    a2 = point[1].x - point[2].x;
	b2 = point[1].y - point[2].y;
	c2 = point[1].z - point[2].z;

	// Then four coefficients become
	p[0] = b1*c2 - b2*c1; 
	p[1] = a2*c1 - a1*c2; 
	p[2] = a1*b2 - a2*b1;
	p[3] = point[2].x * (b2*c1-b1*c2) + point[2].y * (a1*c2-a2*c1) + point[2].z * (a2*b1-a1*b2);
    // Now we got the plane
}

