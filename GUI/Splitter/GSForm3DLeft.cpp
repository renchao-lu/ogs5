// GeoSysForm.cpp : implementation file
//
#include "stdafx.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "COGLPickingView.h"
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
#include "GSForm3DLeft.h"
#include "dtmesh.h"
#include "msh_lib.h"
//FEM
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mmp_new.h"
#include "rf_st_new.h"
#include "gs_pcs_oglcontrol.h"
#include "gs_meshtypechange.h"
#include "gs_project.h"
#include "GSForm3DLeft.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CGLPolyline *geo_polyline = NULL;
vector<CGLPolyline*>::iterator pp = NULL;

Surface *geo_surface = NULL;
vector<Surface*>::iterator ps = NULL;//CC

CGLVolume *geo_volume = NULL;
vector<CGLVolume*>::iterator pv = NULL;

CBoundaryCondition *m_bc = NULL;
list<CBoundaryCondition*>::const_iterator p_bc = NULL;

CMATGroupsControl *m_material_groups = NULL;
vector<CMATGroupsControl*> material_groups_vector;

/////////////////////////////////////////////////////////////////////////////
// CGSForm3DLeft

IMPLEMENT_DYNCREATE(CGSForm3DLeft, CFormView)

CGSForm3DLeft::CGSForm3DLeft()
	: CFormView(CGSForm3DLeft::IDD)
{
    m_TreeImages.Create( IDB_TREE_CTRL_OPTIONS, 16, 1, RGB(0, 255, 0) ); 
	//{{AFX_DATA_INIT(CGSForm3DLeft)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CGSForm3DLeft::~CGSForm3DLeft()
{
}

void CGSForm3DLeft::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGSForm3DLeft)
    
    //DDX_Text(pDX,IDC_EDIT_TOLERANCE, m_tolerancefactor);
	
    //}}AFX_DATA_MAP

  
}

void CGSForm3DLeft::OnDraw(CDC* pDC)
{
    pDC=pDC;//TK
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    if (m_frame->m_rebuild_formtree == 1)
    {
        UpdateTree();
        SetTree();
        m_frame->m_rebuild_formtree = 0;

    }
   
}

BEGIN_MESSAGE_MAP(CGSForm3DLeft, CFormView)
	//{{AFX_MSG_MAP(CGSForm3DLeft)

	//}}AFX_MSG_MAP
    ON_NOTIFY(NM_CLICK, IDC_TREE1, OnTvnSelchangedTree1) //NM_CLICK, TVN_SELCHANGED
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGSForm3DLeft diagnostics

#ifdef _DEBUG
void CGSForm3DLeft::AssertValid() const
{
	CFormView::AssertValid();
}

void CGSForm3DLeft::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGSForm3DLeft message handlers

void CGSForm3DLeft::OnInitialUpdate() 
{
 CFormView::OnInitialUpdate();

 // TREE CONTROL
  SetTree();

}

void CGSForm3DLeft::SetTree()
{
// TREE CONTROL

    int list_length = 0;
    int i=0;
    int j=0;
    CString item_name;


pCtrl = (CTreeCtrl*) GetDlgItem(IDC_TREE1);
pCtrl_copy = pCtrl;
ASSERT(pCtrl != NULL);

pCtrl->DeleteAllItems();

pCtrl->SetImageList(&m_TreeImages, TVSIL_NORMAL ); 

TVINSERTSTRUCT tvInsert;
tvInsert.hParent = NULL;
tvInsert.hInsertAfter = NULL;
tvInsert.item.mask = TVIF_TEXT;
/*GEO TREE - GEO TREE - GEO TREE*/
tvInsert.item.pszText = _T("GEO");
hGEOMETRY = pCtrl->InsertItem("GEO",9,9);

/*Point TREE*/ 
hPOINTS = pCtrl->InsertItem(_T("Points"), 0, 1,hGEOMETRY);
    point_list_length = (int)gli_points_vector.size();
    list_length = point_list_length;
    for (i=0;i<list_length;i++)
    {
        item_name.Format(_T("%d"),gli_points_vector[i]->id);//CC
        _tprintf(_T("%s"), (LPCTSTR) item_name);
        pCtrl->InsertItem(item_name,0,1, hPOINTS, TVI_LAST);
        gli_points_vector[i]->display_mode = 0;
    }
/*Line TREE*/ 
hLINES = pCtrl->InsertItem(_T("Lines"), 0, 1,hGEOMETRY);
    line_list_length = (int)gli_file_lines_vector.size();
    list_length = line_list_length;
    for (i=0;i<list_length;i++)
    {
        item_name.Format(_T("%d"),gli_file_lines_vector[i]->gli_line_id);
        _tprintf(_T("%s"), (LPCTSTR) item_name);
        pCtrl->InsertItem(item_name,0,1, hLINES, TVI_LAST);
        gli_file_lines_vector[i]->display_mode = 0;
    }
/*Polyline TREE*/ 
    HTREEITEM hPOLYLINES = pCtrl->InsertItem(_T("Polylines"),0, 1, hGEOMETRY);
    pp = polyline_vector.begin(); //CC
    list_length = (int)polyline_vector.size();//CC
    for (i=0;i<list_length;i++)
    {
        geo_polyline = *pp;
        pCtrl->InsertItem(geo_polyline->name.data(),0,1, hPOLYLINES, TVI_LAST);
        pp++;
    }

/*Surface TREE*/ 
    HTREEITEM hSURFACES = pCtrl->InsertItem(_T("Surfaces"), 0, 1, hGEOMETRY);
    Surface *gl_surface = NULL;
    vector<Surface*>::iterator ps = surface_vector.begin();
    list_length = (int)surface_vector.size();//CC
    for (i=0;i<list_length;i++)
    {
        gl_surface = *ps;
        pCtrl->InsertItem(gl_surface->name.data(),0,1, hSURFACES, TVI_LAST);
        ps++;
    }

/*Volume TREE*/ 
    HTREEITEM hVOLUMES = pCtrl->InsertItem(_T("Volumes"),0, 1, hGEOMETRY);
    CGLVolume *gl_volume = NULL;
      vector<CGLVolume*>::iterator p_vol = volume_vector.begin();//CC
    list_length  = (int)volume_vector.size();  
    for(i=0;i<list_length;i++)
    {
     gl_volume = *p_vol;
     pCtrl->InsertItem(gl_volume->name.data(),0,1, hVOLUMES, TVI_LAST);
     ++p_vol;
    }

/*MSH TREE - MSH TREE - MSH TREE*/
tvInsert.item.pszText = _T("MSH");
HTREEITEM hMESH = pCtrl->InsertItem("MSH",9,9);

    /*PCS TREE*/ 
    list_length  =  (int)fem_msh_vector.size();
    for (i=0;i<list_length;i++)
    {
        item_name.Format(_T("%d"),i);
        _tprintf(_T("%s"), (LPCTSTR) item_name);
        pCtrl->InsertItem(fem_msh_vector[i]->pcs_name.data(),10,10, hMESH, TVI_LAST);

    /*Nodes */ 
    item_name.Format(_T("%d"),fem_msh_vector[i]->nod_vector.size());
    _tprintf(_T("%s"), (LPCTSTR) item_name);
    item_name = "Nodes (" + item_name + ")";
    pCtrl->InsertItem(item_name, 0, 1, hMESH);

    /*Elements */ 
    item_name.Format(_T("%d"),fem_msh_vector[i]->ele_vector.size());
    _tprintf(_T("%s"), (LPCTSTR) item_name);
    item_name = "Elements (" + item_name + ")";
    pCtrl->InsertItem(item_name, 0, 1, hMESH);

    
    /*Material Group Numbers*/ 
    HTREEITEM hMATGROUPNB = pCtrl->InsertItem(_T("Material Groups"), 0, 1, hMESH);
    if((int)fem_msh_vector[i]->ele_vector.size()>0)
    fem_msh_vector[i]->highest_mat_group_nb = fem_msh_vector[i]->ele_vector[0]->GetPatchIndex();
    for (j=0;j<(int)fem_msh_vector[i]->ele_vector.size();j++)
    {
        
        if (fem_msh_vector[i]->highest_mat_group_nb < fem_msh_vector[i]->ele_vector[j]->GetPatchIndex())
        fem_msh_vector[i]->highest_mat_group_nb = fem_msh_vector[i]->ele_vector[j]->GetPatchIndex();
    }

    for (j=0;j<=(int)fem_msh_vector[i]->highest_mat_group_nb;j++)
    {
        item_name.Format(_T("%d"),j);
        _tprintf(_T("%s"), (LPCTSTR) item_name);

        if(j < (int)mmp_vector.size())
        item_name = item_name + " (" + mmp_vector[j]->name.data() + ")";
        else         
        item_name = item_name + " (?)";
        pCtrl->InsertItem(item_name,0,1, hMATGROUPNB, TVI_LAST);
    }
        /*Material Group Properties*/
        pCtrl->InsertItem("Properties",10,10, hMATGROUPNB, TVI_LAST);
        pCtrl->InsertItem("MG_Mesh",0,1, hMATGROUPNB, TVI_LAST);
        pCtrl->InsertItem("X_Value",0,1, hMATGROUPNB, TVI_LAST);
        pCtrl->InsertItem("Y_Value",0,1, hMATGROUPNB, TVI_LAST);
        pCtrl->InsertItem("Z_Value",0,1, hMATGROUPNB, TVI_LAST);
    }


/*FEM TREE - FEM TREE - FEM TREE*/
tvInsert.item.pszText = _T("PCS");
HTREEITEM hFEM = pCtrl->InsertItem("PCS",9,9);

    /*PCS TREE*/   
    CRFProcess* m_process = NULL;
    list_length = (int)pcs_vector.size();
    for(j=0;j<list_length;j++)
    {
      m_process = pcs_vector[j];
      item_name = m_process->pcs_type_name.data();
      pCtrl->InsertItem(item_name,10,10, hFEM, TVI_LAST);

          item_name = (CString)m_process->pcs_primary_function_name[0];
          pCtrl->InsertItem(item_name,0,1, hFEM, TVI_LAST);          
  

        /*BC TREE*/ 
        CBoundaryConditionsGroup *m_bc_group = NULL;
        m_bc_group = m_bc_group->Get(m_process->pcs_type_name.data());
        item_name.Format(_T("%d"),m_bc_group->group_vector.size());
        _tprintf(_T("%s"), (LPCTSTR) item_name);
        item_name = "BC (" + item_name + " nodes)";
        pCtrl->InsertItem(item_name, 0, 1, hFEM);

        /*ST TREE*/ 
        HTREEITEM hSST = pCtrl->InsertItem(_T("SST"),0,1, hFEM);
        int st_list_length  =  (int)st_vector.size();       
        for (i=0;i<st_list_length;i++)
        {         
            item_name.Format(_T("%d"),st_vector[i]->geo_id);
            _tprintf(_T("%s"), (LPCTSTR) item_name);
            item_name = "LIN:" + item_name; 
            pCtrl->InsertItem(item_name,0,1, hSST, TVI_LAST);      
        }

      }
/*Material Group TREE*/
HTREEITEM hMAT = pCtrl->InsertItem("MAT",9,9);

    HTREEITEM hMATGROUPS = pCtrl->InsertItem(_T("Material Groups"), 0, 1, hMAT);
    list_length  =  (int)mmp_vector.size();
    
        for (j=0;j<(int)material_groups_vector.size();j++){
             delete material_groups_vector[j];//deletes vector
        }
        material_groups_vector.clear();

    for (i=0;i<list_length;i++)
    {

        m_material_groups = new CMATGroupsControl;
        m_material_groups->materialnumber= mmp_vector[i]->number;
        m_material_groups->display_mode = 0;
        material_groups_vector.push_back(m_material_groups);


        mmp_vector[i]->number;
        item_name.Format(_T("%d"),mmp_vector[i]->number);
        _tprintf(_T("%s"), (LPCTSTR) item_name);
        pCtrl->InsertItem(item_name,0,1, hMATGROUPS, TVI_LAST);

    }


}

void CGSForm3DLeft::OnDataChange()
{
    	if (!UpdateData())
		return;

    CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();    
    mainframe->m_3dcontrol_points = m_3dcontrol_points;
    mainframe->m_3dcontrol_lines = m_3dcontrol_lines;
    mainframe->m_3dcontrol_polylines = m_3dcontrol_polylines;
    mainframe->m_3dcontrol_surfaces = m_3dcontrol_surfaces;     
    mainframe->m_3dcontrol_volumes = m_3dcontrol_volumes;     
    mainframe->m_3dcontrol_nodes = m_3dcontrol_nodes;     
    mainframe->m_3dcontrol_elements = m_3dcontrol_elements;
    mainframe->m_3dcontrol_matgroups = m_3dcontrol_matgroups;   
    mainframe->m_3dcontrol_bc = m_3dcontrol_bc;
    mainframe->m_3dcontrol_pcs = m_3dcontrol_pcs;
    mainframe->m_3dcontrol_sourcesink = m_3dcontrol_sourcesink;
    mainframe->m_pcs_name = m_pcs_name;
    mainframe->m_selected_wire_frame = m_selected_wire_frame;   
    mainframe->m_x_value_color = m_x_value_color;
    mainframe->m_y_value_color = m_y_value_color;
    mainframe->m_z_value_color = m_z_value_color;

    mainframe->m_something_changed = 1;
    CGeoSysDoc* pdoc = GetDocument();
    pdoc->UpdateAllViews(this);

}


void CGSForm3DLeft::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
  int nImage;
  int nSelectedImage;
  long size;
  int i=0;
  int j=0;
  long id;
  CString item_name;
  CString pcs_item_name;
  Cgs_pcs_oglcontrol* m_modelessDlg=NULL;
  LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
  pNMTreeView=pNMTreeView;//TK
  pCtrl = (CTreeCtrl*) GetDlgItem(IDC_TREE1);
  ASSERT(pCtrl != NULL);

   
  HTREEITEM hItem=NULL ;
  HTREEITEM hChildItem=NULL;
  HTREEITEM hParentItem=NULL;
  HTREEITEM hPCSItem=NULL;
  hItem = pCtrl->GetNextItem(hItem, TVGN_CARET);
  item_name = pCtrl->GetItemText(hItem);

  CPoint point;
  UINT uFlags=0;
  DWORD dwpos = GetMessagePos();
  dwpos=dwpos;
  point = GetCurrentMessage()->pt;
  
  ScreenToClient(&point);
  point.x = point.x -13;
  point.y = point.y -13;
  hItem = pCtrl->HitTest(point, &uFlags);

  if ((uFlags & TVHT_ONITEM) || (uFlags & TVHT_ONITEMINDENT) || (uFlags & TVHT_ONITEMRIGHT))
	{
        if (IsCheckBox(hItem))
        {
        BOOL bEnable;
        VERIFY(GetCheckBoxEnable(hItem, bEnable));
        if (bEnable)
        {
            //Toggle the state of check items
            BOOL bCheck;
            VERIFY(GetCheckBox(hItem, bCheck));
            VERIFY(SetCheckBox(hItem, !bCheck));
        }

        }

        item_name = pCtrl->GetItemText(hItem);
        BOOL bSuccess = pCtrl->GetItemImage(hItem, nImage, nSelectedImage);
        bSuccess=bSuccess;

/*PARENT*/         

        /*POINTS*/   
        if (nSelectedImage == 1 && item_name == "Points")
        {
             m_3dcontrol_points = 1;
             /*---------------------------------------*/ 
             size = (int)gli_points_vector.size();
             for (i=0; i<size; i++)
             {
               gli_points_vector[i]->display_mode = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
             }
             /*---------------------------------------*/ 
             OnDataChange();       
			 // PCH 
			 theApp.GLINodeSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name == "Points")
        {
              m_3dcontrol_points = 1;

             /*---------------------------------------*/ 
             size = (int)gli_points_vector.size();
             for (i=0; i<size; i++)
             {
               gli_points_vector[i]->display_mode = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
             }
             /*---------------------------------------*/ 
              OnDataChange();       
			 // PCH 
			 theApp.GLINodeSwitch = 0;
        }
//
        /*LINES*/   
        if (nSelectedImage == 1 && item_name == "Lines")
        {
             m_3dcontrol_lines = 1;
             /*---------------------------------------*/ 
             size = (int)gli_file_lines_vector.size();
             for (i=0; i<size; i++)
             {
               gli_file_lines_vector[i]->display_mode = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
             }
             /*---------------------------------------*/ 
             OnDataChange();       
        }
        if (nSelectedImage == 0 && item_name == "Lines")
        {
              m_3dcontrol_lines = 1;

             /*---------------------------------------*/ 
             size = (int)gli_file_lines_vector.size();
             for (i=0; i<size; i++)
             {
               gli_file_lines_vector[i]->display_mode = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
             }
             /*---------------------------------------*/ 
              OnDataChange();       
        }
        /*POLYLINES*/   
        if (nSelectedImage == 1 && item_name == "Polylines")
        {
             m_3dcontrol_polylines = 1;
             /*---------------------------------------*/ 
             pp = polyline_vector.begin();//CC
             size = (int)polyline_vector.size();//CC
             for (i=0; i<size; i++)
             {
               geo_polyline = *pp;
               geo_polyline ->display_mode = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
               ++pp;
             }
             /*---------------------------------------*/ 
             OnDataChange();       
			 // PCH 
			 theApp.PolylineSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name == "Polylines")
        {
              m_3dcontrol_polylines = 0;
             /*---------------------------------------*/ 
             pp = polyline_vector.begin();//CC
             size = (int)polyline_vector.size();//CC
             for (i=0; i<size; i++)
             {
               geo_polyline = *pp;
               geo_polyline ->display_mode = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
               ++pp;
             }
             /*---------------------------------------*/ 
              OnDataChange();       
			  // PCH 
			 theApp.PolylineSwitch = 0;
        }

		// PCH /*Polylines*/
		// When one of polyline is selected, that event will select or deselect the polyline.
		// For selection first
		if (nSelectedImage == 1 && theApp.PolylineSwitch == 1)
		{
			//DoThisPolyline(item_name, hItem, &hChildItem);
            //TODO: It chrashs when you try to click Polyline and Surfaces
		}
		else if(nSelectedImage == 0 && theApp.PolylineSwitch == 1)
		{
			// Deselect polyline
			theApp.hitsPolylineTotal = 0;
		}
		// PCH Ends here.

        /*SURFACES*/   
        if (nSelectedImage == 1 && item_name == "Surfaces")
        {
             m_3dcontrol_surfaces = 1;
             /*---------------------------------------*/ 
             ps = surface_vector.begin();
             size = (long)surface_vector.size();//CC
             for (i=0; i<size; i++)
             {
               geo_surface = *ps;
               geo_surface ->display_mode_3d = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
               ++ps;
             }
             /*---------------------------------------*/ 
             OnDataChange();       
			 // PCH 
			 theApp.SurfaceSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name == "Surfaces")
        {
              m_3dcontrol_surfaces = 0;
             /*---------------------------------------*/ 
             ps = surface_vector.begin();
             size = (long)surface_vector.size();//CC
             for (i=0; i<size; i++)
             {
               geo_surface = *ps;
               geo_surface ->display_mode_3d = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
               ++ps;
             }
             /*---------------------------------------*/ 
              OnDataChange();       
			  // PCH 
			  theApp.SurfaceSwitch = 0;
        }

        /*VOLUMES*/   
        if (nSelectedImage == 1 && item_name == "Volumes")
        {
             m_3dcontrol_volumes = 1;
             /*---------------------------------------*/ 
             pv = volume_vector.begin();
             size = (int)volume_vector.size();
             for (i=0; i<size; i++)
             {
               geo_volume = *pv;
               geo_volume ->display_mode = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
               ++pv;
             }
             /*---------------------------------------*/ 
             OnDataChange();       
			 // PCH 
			 theApp.VolumeSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name == "Volumes")
        {
              m_3dcontrol_volumes = 0;
             /*---------------------------------------*/ 
             pv = volume_vector.begin();
             size = (int)volume_vector.size();
             for (i=0; i<size; i++)
             {
               geo_volume = *pv;
               geo_volume ->display_mode = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
               ++pv;
             }
             /*---------------------------------------*/ 
              OnDataChange();       
			  // PCH 
			 theApp.VolumeSwitch = 0;
        }

        /*NODES*/      
        if (nSelectedImage == 1 && item_name.Find("Nodes") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
             size = (long)fem_msh_vector.size();
             for (i=0; i<size; i++)
             {
                 if (fem_msh_vector[i]->pcs_name.data() == pcs_item_name)
                   fem_msh_vector[i]->nod_display_mode= 1;
             }
             //m_3dcontrol_nodes = 1;
             OnDataChange();       
			 // PCH 
			 theApp.RFINodeSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name.Find("Nodes") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
             size = (long)fem_msh_vector.size();
             for (i=0; i<size; i++)
             {
                 if (fem_msh_vector[i]->pcs_name.data() == pcs_item_name)
                   fem_msh_vector[i]->nod_display_mode = 0;
             }
             //m_3dcontrol_nodes = 0;
             OnDataChange();       
			 // PCH 
			 theApp.RFINodeSwitch = 0;
        }
        /*ELEMENTS*/   
        if (nSelectedImage == 1 && item_name.Find("Elements") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
             size = (long)fem_msh_vector.size();
             for (i=0; i<size; i++)
             {
                 if (fem_msh_vector[i]->pcs_name.data() == pcs_item_name)
                   fem_msh_vector[i]->ele_display_mode= 1;
             }
             //m_3dcontrol_elements = 1;
             OnDataChange();       
			 // PCH 
			 theApp.ElementSwitch = 1;
        }
        if (nSelectedImage == 0 && item_name.Find("Elements") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
             size = (long)fem_msh_vector.size();
             for (i=0; i<size; i++)
             {
                 if (fem_msh_vector[i]->pcs_name.data() == pcs_item_name)
                   fem_msh_vector[i]->ele_display_mode= 0;
             }
             //m_3dcontrol_elements = 0;
             OnDataChange();       
     		 // PCH 
			 theApp.ElementSwitch = 0;
        }


        /*Material Groups*/   
        if (nSelectedImage == 1 && item_name.Find("Material Groups") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
             for (j=0; j<(long)fem_msh_vector.size(); j++)
             {
                 if (fem_msh_vector[j]->pcs_name.data() == pcs_item_name)
                 {
                    fem_msh_vector[j]->ele_mat_display_mode = 1;
                    for (i=0; i<=(int)fem_msh_vector[j]->highest_mat_group_nb; i++)
                    {
                    if (i==0 && pCtrl->ItemHasChildren(hItem)) 
                    {
                        hChildItem = pCtrl->GetChildItem(hItem);
                        SetCheckBox(hChildItem, TRUE);
                    }
                    if (i>0 && pCtrl->ItemHasChildren(hItem)) 
                    {
                        hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                        string name = pCtrl->GetItemText(hChildItem);
                        SetCheckBox(hChildItem, TRUE);
                    }         
                    }
                    for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
                    {
                      fem_msh_vector[j]->ele_vector[i]->matgroup_view = 1;
                    }
                 }
             }
             OnDataChange();       
        }
        if (nSelectedImage == 0 && item_name.Find("Material Groups") == 0)
        {
             hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);
            
             for (j=0; j<(long)fem_msh_vector.size(); j++)
             {
                 if (fem_msh_vector[j]->pcs_name.data() == pcs_item_name)
                 {
                    fem_msh_vector[j]->ele_mat_display_mode = 0;
                    for (i=0; i<=(int)fem_msh_vector[j]->highest_mat_group_nb; i++)
                    {
                    if (i==0 && pCtrl->ItemHasChildren(hItem)) 
                    {
                        hChildItem = pCtrl->GetChildItem(hItem);
                        SetCheckBox(hChildItem, FALSE);
                    }
                    if (i>0 && pCtrl->ItemHasChildren(hItem)) 
                    {
                        hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                        SetCheckBox(hChildItem, FALSE);
                    }         
                    }
                    for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
                    {
                      fem_msh_vector[j]->ele_vector[i]->matgroup_view = 0;
                    }

                 }
             }
             OnDataChange();       
        }



        /*MATERIAL PROPERTIES*/   
        if (nSelectedImage == 1 && item_name == "Material Properties")
        {
 
             m_3dcontrol_matgroups = 1;           
             size = (int)material_groups_vector.size();
             for (i=0; i<size; i++)
             {
               material_groups_vector[i]->display_mode = 1;
               
               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
             
             }
             /*---------------------------------------*/ 
             OnDataChange();       
        }
        if (nSelectedImage == 0 && item_name == "Material Properties")
        {
             m_3dcontrol_matgroups = 0;           
             size = (int)material_groups_vector.size();
             for (i=0; i<size; i++)
             {
               material_groups_vector[i]->display_mode = 0;              

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
               
             }
             /*---------------------------------------*/ 
              OnDataChange();       
        }
        /*SourceSinkTerms*/   
        if (nSelectedImage == 1 && item_name == "SST")
        {
             m_3dcontrol_sourcesink = 1;
             /*---------------------------------------*/ 
             size = (int)st_vector.size();
             for (i=0; i<size; i++)
             {
               st_vector[i]->display_mode = 1;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
             }
             /*---------------------------------------*/ 
             OnDataChange();       
        }
        if (nSelectedImage == 0 && item_name == "SST")
        {
              m_3dcontrol_sourcesink = 1;

             /*---------------------------------------*/ 
             size = (int)st_vector.size();
             for (i=0; i<size; i++)
             {
               st_vector[i]->display_mode = 0;

               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
             }
             /*---------------------------------------*/ 
              OnDataChange();       
        }
        /*Boundary Conditions*/         
        if (nSelectedImage == 1 && item_name.Find("BC ") == 0)
        {
            hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
            hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
            pcs_item_name = pCtrl->GetItemText(hPCSItem);

            CBoundaryConditionsGroup *m_bc_group = NULL;
            m_bc_group = m_bc_group->Get((string)pcs_item_name);
            m_bc_group->m_display_mode_bc = 1;
            m_3dcontrol_bc = 1; 
             /*---------------------------------------*/ 
               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, TRUE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, TRUE);
               }
             /*---------------------------------------*/ 
             OnDataChange();       
        }
        if (nSelectedImage == 0 && item_name.Find("BC ") == 0)
        {
            hPCSItem = pCtrl->GetNextItem(hItem, TVGN_PREVIOUS);
            hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
            pcs_item_name = pCtrl->GetItemText(hPCSItem);

            CBoundaryConditionsGroup *m_bc_group = NULL;
            m_bc_group = m_bc_group->Get((string)pcs_item_name);
            m_bc_group->m_display_mode_bc = 0;
            m_3dcontrol_bc = 0;
             /*---------------------------------------*/          
               if (i==0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetChildItem(hItem);
                   SetCheckBox(hChildItem, FALSE);
               }
               if (i>0 && pCtrl->ItemHasChildren(hItem)) 
               {
                   hChildItem = pCtrl->GetNextItem(hChildItem, TVGN_NEXT);
                   SetCheckBox(hChildItem, FALSE);
               }
             /*---------------------------------------*/ 
              OnDataChange();       
        }



/*CHILDREN*/         

      if (nSelectedImage == 1)
        {
            hParentItem = pCtrl->GetParentItem(hItem);
            item_name = pCtrl->GetItemText(hParentItem);
            /*POINTS*/      
            if (item_name == "Points")
            {
                m_3dcontrol_points = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                gli_points_vector[id]->display_mode = 1;

                OnDataChange();
            }                   
            /*Lines*/      
            if (item_name == "Lines")
            {
                m_3dcontrol_lines = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                gli_file_lines_vector[id]->display_mode = 1;

                OnDataChange();
            }                   
 
            /*POLYLINES*/      
            if (item_name == "Polylines")
            {
                m_3dcontrol_polylines = 1;

                item_name = pCtrl->GetItemText(hItem);
                pp = polyline_vector.begin();//CC
                size = (int)polyline_vector.size();//CC
                for (i=0; i<size; i++)
                {
                   geo_polyline = *pp;
                   if (item_name == geo_polyline->name.data())
                   {
                       geo_polyline ->display_mode = 1;
                   }
                   ++pp;
                }
                OnDataChange();
            }                   
            /*SURFACES*/      
            if (item_name == "Surfaces")
            {
                m_3dcontrol_surfaces = 1;

                item_name = pCtrl->GetItemText(hItem);
                ps = surface_vector.begin();
                size = (long)surface_vector.size();//CC
                for (i=0; i<size; i++)
                {
                   geo_surface = *ps;
                   if (item_name == geo_surface->name.data())
                   {
                       geo_surface ->display_mode_3d = 1;
                   }
                   ++ps;
                }
                OnDataChange();
            }                   
            /*VOLUMES*/      
            if (item_name == "Volumes")
            {
                m_3dcontrol_volumes = 1;

                item_name = pCtrl->GetItemText(hItem);
                pv = volume_vector.begin();
                size = (int)volume_vector.size();
                for (i=0; i<size; i++)
                {
                   geo_volume = *pv;
                   if (item_name == geo_volume->name.data())
                   {
                       geo_volume ->display_mode = 1;
                   }
                   ++pv;
                }
                OnDataChange();
            }                   
            
            /*MATERIAL GROUPS*/      
            if (item_name == "Material Groups")
            {
             hPCSItem = pCtrl->GetNextItem(hParentItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);

             item_name = pCtrl->GetItemText(hItem);
             id = atol(item_name);

             if (item_name == "MG_Mesh" || item_name == "X_Value" ||
                 item_name == "Y_Value" || item_name == "Z_Value" )
             {
                if (item_name == "MG_Mesh" )m_selected_wire_frame = 1;
                if (item_name == "X_Value" )m_x_value_color = 1;
                if (item_name == "Y_Value" )m_y_value_color = 1;
                if (item_name == "Z_Value" )m_z_value_color = 1;
             }
             else
             {
             for (j=0; j<(long)fem_msh_vector.size(); j++)
             {
                 if (fem_msh_vector[j]->pcs_name.data() == pcs_item_name)
                 {
                    fem_msh_vector[j]->ele_mat_display_mode = 1;
                    for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
                    {
                      if (fem_msh_vector[j]->ele_vector[i]->GetPatchIndex() == id)
                      fem_msh_vector[j]->ele_vector[i]->matgroup_view = 1;
                    }
                 }
             }
             }
                OnDataChange();
            }


            /*MATERIAL GROUPS*/      
            if (item_name == "M-Groups")//TODO
            {
                m_3dcontrol_matgroups = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                material_groups_vector[id]->display_mode = 1;

                OnDataChange();
            }

            /*PCS*/      
            if (item_name == "PCS")
            {
                CString parent_item_name = pCtrl->GetItemText(hItem);
                if (parent_item_name.Find("BC ")!=0 && parent_item_name != "SST")
                {
                    m_3dcontrol_pcs = 1;
                    m_pcs_name = parent_item_name;

                    OnDataChange();
	                if(m_modelessDlg==NULL)
	                {
		                m_modelessDlg=new Cgs_pcs_oglcontrol(this);
		                m_modelessDlg->Create(IDD_PCS_DISPLAY_CONTROL);
		                m_modelessDlg->ShowWindow(SW_SHOW);
                        m_modelessDlg->BringWindowToTop();
                        long cx = ::GetSystemMetrics(SM_CXMAXIMIZED);
                        long cy = ::GetSystemMetrics(SM_CYMAXIMIZED);
                        cx = 200;
                        cy = 200;
                        long x =  ::GetSystemMetrics(SM_CXMAXIMIZED)- cx;
                        long y =  ::GetSystemMetrics(SM_CYMAXIMIZED)- cy ;
                        m_modelessDlg->SetWindowPos(&CWnd::wndTop,x,y,cx,cy,SWP_NOSIZE);
                    }
                }              
            }                   


            /*Source Sink Terms*/      
            if (item_name == "SST")
            {
                m_3dcontrol_sourcesink = 1;

                item_name = pCtrl->GetItemText(hItem);
                int string_length = item_name.GetLength();
                item_name = item_name.Right(string_length-4);
                id = atol(item_name);
                st_vector[id]->display_mode = 1;

                OnDataChange();
            }

        }
        if (nSelectedImage == 0)
        {
            hParentItem = pCtrl->GetParentItem(hItem);
            item_name = pCtrl->GetItemText(hParentItem);
            
            /*POINTS*/ 
            if (item_name == "Points")
            {
                m_3dcontrol_points = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                gli_points_vector[id]->display_mode = 0;

                OnDataChange();
            }        
            /*LINES*/ 
            if (item_name == "Lines")
            {
                m_3dcontrol_lines = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                gli_file_lines_vector[id]->display_mode = 0;

                OnDataChange();
            }        
            /*POLYLINES*/      
            if (item_name == "Polylines")
            {
                m_3dcontrol_polylines = 1;

                item_name = pCtrl->GetItemText(hItem);
                pp = polyline_vector.begin();//CC
                size = (int)polyline_vector.size();//CC
                for (i=0; i<size; i++)
                {
                   geo_polyline = *pp;
                   if (item_name == geo_polyline->name.data())
                   {
                       geo_polyline ->display_mode = 0;
                   }
                   ++pp;
                }
                OnDataChange();
            }                   
            /*SURFACES*/      
            if (item_name == "Surfaces")
            {
                m_3dcontrol_surfaces = 1;

                item_name = pCtrl->GetItemText(hItem);
                ps = surface_vector.begin();
                size = (long)surface_vector.size();//CC
                for (i=0; i<size; i++)
                {
                   geo_surface = *ps;
                   if (item_name == geo_surface->name.data())
                   {
                       geo_surface ->display_mode_3d = 0;
                   }
                   ++ps;
                }
                OnDataChange();
            }                   
            /*VOLUMES*/      
            if (item_name == "Volumes")
            {
                m_3dcontrol_volumes = 1;

                item_name = pCtrl->GetItemText(hItem);
                pv = volume_vector.begin();
                size = (int)volume_vector.size();
                for (i=0; i<size; i++)
                {
                   geo_volume = *pv;
                   if (item_name == geo_volume->name.data())
                   {
                       geo_volume ->display_mode = 0;
                   }
                   ++pv;
                }
                OnDataChange();
            }

            /*MATERIAL GROUPS*/      
            if (item_name == "Material Groups")
            {
             hPCSItem = pCtrl->GetNextItem(hParentItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             hPCSItem = pCtrl->GetNextItem(hPCSItem, TVGN_PREVIOUS);
             pcs_item_name = pCtrl->GetItemText(hPCSItem);

             item_name = pCtrl->GetItemText(hItem);
             id = atol(item_name);
             if (item_name == "MG_Mesh" || item_name == "X_Value" ||
                 item_name == "Y_Value" || item_name == "Z_Value" )
             {
                if (item_name == "MG_Mesh" )m_selected_wire_frame = 0;
                if (item_name == "X_Value" )m_x_value_color = 0;
                if (item_name == "Y_Value" )m_y_value_color = 0;
                if (item_name == "Z_Value" )m_z_value_color = 0;
             }
             else
             {
             for (j=0; j<(long)fem_msh_vector.size(); j++)
             {
                 if (fem_msh_vector[j]->pcs_name.data() == pcs_item_name)
                 {
                    fem_msh_vector[j]->ele_mat_display_mode = 1;
                    for (i=0; i<(int)fem_msh_vector[j]->ele_vector.size(); i++)
                    {
                      if (fem_msh_vector[j]->ele_vector[i]->GetPatchIndex() == id)
                      fem_msh_vector[j]->ele_vector[i]->matgroup_view = 0;
                    }
                 }
             }
             }
                OnDataChange();
            }

            /*MATERIAL GROUPS*/      
            if (item_name == "M-Groups")
            {
                m_3dcontrol_matgroups = 1;

                item_name = pCtrl->GetItemText(hItem);
                id = atol(item_name);
                material_groups_vector[id]->display_mode = 0;

                OnDataChange();
            }

            /*PCS*/      
            if (item_name == "PCS")
            {
                CString parent_item_name = pCtrl->GetItemText(hItem);
                if (parent_item_name.Find("BC ")!=0 )
                {
                    m_3dcontrol_pcs = 0;
                    m_pcs_name = parent_item_name;
                    OnDataChange();
                }
              
            }               

            /*SOURCE SINK TERMS*/      
            if (item_name == "SST")
            {
                m_3dcontrol_sourcesink = 1;

                item_name = pCtrl->GetItemText(hItem);
                int string_length = item_name.GetLength();
                item_name = item_name.Right(string_length-4);
                id = atol(item_name);
                st_vector[id]->display_mode = 0;

                OnDataChange();
            }

        }


         item_name = pCtrl->GetItemText(hItem);


    }    
   pResult=0;
}

BOOL CGSForm3DLeft::GetCheckBox(HTREEITEM hItem, BOOL& bCheck)
{
  //Validate our parameters
  ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = pCtrl->GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bCheck = (nImage == 1 || nImage == 5);

  return bSuccess;
}

BOOL CGSForm3DLeft::SetCheckBox(HTREEITEM hItem, BOOL bCheck)
{
  //Validate our parameters
  ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it

  BOOL bSuccess;
  if (bCheck)
    bSuccess = pCtrl->SetItemImage(hItem, 1, 1);
  else
    bSuccess = pCtrl->SetItemImage(hItem, 0, 0);   
  
  return bSuccess;
}


BOOL CGSForm3DLeft::IsCheckBox(HTREEITEM hItem)
{
  int nImage;
  int nSelectedImage;
  BOOL bSuccess = pCtrl->GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  return (nImage == 0 || nImage == 1 || nImage == 4 || nImage == 5);
}

BOOL CGSForm3DLeft::SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable)
{
  ASSERT(IsCheckBox(hItem)); //Must be a check box
  BOOL bSuccess = FALSE;

  if (bEnable)
  {
    BOOL bCheck;
    VERIFY(GetCheckBox(hItem, bCheck));
    if (bCheck)
      bSuccess = pCtrl->SetItemImage(hItem, 1, 1);
    else
      bSuccess = pCtrl->SetItemImage(hItem, 0, 0);
  }
  else
  {
    BOOL bCheck;
    VERIFY(GetCheckBox(hItem, bCheck));
    if (bCheck)
      bSuccess = pCtrl->SetItemImage(hItem, 5, 5);
    else
      bSuccess = pCtrl->SetItemImage(hItem, 4, 4);
  }

  return bSuccess;
}

BOOL CGSForm3DLeft::GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable)
{
  ASSERT(IsCheckBox(hItem)); //Must be a check box

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = pCtrl->GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bEnable = (nImage == 0 || nImage == 1);

  return bSuccess;  
}




void CGSForm3DLeft::showChange()
{
	// Update the change by redrawing
	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;

	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

	// Get the active view attached to the active MDI child window.
	COGLPickingView *pView = (COGLPickingView *) pChild->GetActiveView();
	pView->Invalidate();
}

void CGSForm3DLeft::DoThisPolyline(CString item_name, HTREEITEM hItem, HTREEITEM* hChildItem)
{
	// Loop over the number of polylines
	pp = polyline_vector.begin();//CC
	
	for (int i=0;i<(int)polyline_vector.size();i++)//CC
	{
		geo_polyline = *pp;
		CString thisPolylineName = geo_polyline->name.data();
				
		// Let's compare
		if(item_name == thisPolylineName)
		{
			int indexOfPolyline = i;

			// Select this polyline
			// Memory allocation for the new selection
			theApp.hitsPolylineTotal = 1;
			theApp.polylinePickedTotal = (
				int *) realloc(theApp.polylinePickedTotal, 
				theApp.hitsPolylineTotal * sizeof(int));
			theApp.polylinePickedTotal[0] = indexOfPolyline;
		}
		// For the other polylines, just uncheck them.
		else 
		{
			if (i==0 && pCtrl->ItemHasChildren(hItem)) 
            {
				*hChildItem = pCtrl->GetChildItem(hItem);
				SetCheckBox(*hChildItem, FALSE);
            }
            if (i>0 && pCtrl->ItemHasChildren(hItem)) 
            {
				*hChildItem = pCtrl->GetNextItem(*hChildItem, TVGN_NEXT);
				SetCheckBox(*hChildItem, FALSE);
            }
		}

		// Next polyline in the list
		pp++;
	}

	// Now, the picked polyline can be further used to select rfi nodes in case later on.
	// Suraces and volumes can also be selected in a similar way. 
	// In this way, we can monitor primal variables in the interested whatever geometry 
	// during simulation.
}



void CGSForm3DLeft::UpdateTree()
{
// TREE CONTROL

int list_length = 0;
int i=0;
int j=0;
CString item_name;

BOOL CheckBoxOn;
HTREEITEM GEOItem = NULL;
HTREEITEM MSHItem = NULL;
HTREEITEM FEMItem = NULL;
HTREEITEM MATItem = NULL;
HTREEITEM Top_Item = NULL;
HTREEITEM First_Level_Item = NULL;
HTREEITEM Second_Level_Item = NULL;
CString item_text;

pCtrl = (CTreeCtrl*) GetDlgItem(IDC_TREE1);
pCtrl_copy = pCtrl;
ASSERT(pCtrl != NULL);

int nImage;
int nSelectedImage;
BOOL bSuccess;

/*Three Level TREE LOOP - TREE - TREE - TREE*/
Top_Item = pCtrl->GetNextItem(Top_Item, TVGN_FIRSTVISIBLE);
while (pCtrl->GetNextItem(Top_Item, TVGN_NEXT))
{
item_text = pCtrl->GetItemText(Top_Item);
bSuccess = pCtrl->GetItemImage(Top_Item, nImage, nSelectedImage);

First_Level_Item = pCtrl->GetNextItem(Top_Item, TVGN_CHILD);
item_text = pCtrl->GetItemText(First_Level_Item);
bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);
if ((nImage==0 || nImage==1) && First_Level_Item != NULL) GetCheckBox(First_Level_Item, CheckBoxOn);
    while (pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT))
    {
    First_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT);
    item_text = pCtrl->GetItemText(First_Level_Item);
    bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);

    Second_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_CHILD);
    item_text = pCtrl->GetItemText(Second_Level_Item);
    bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);

    while (pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT))
    {
        Second_Level_Item = pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT);
        item_text = pCtrl->GetItemText(Second_Level_Item);
        bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);
    }
    }
  Top_Item = pCtrl->GetNextItem(Top_Item, TVGN_NEXT);
}
/*Three Level TREE LOOP - TREE - TREE - TREE*/








/*GEO TREE - GEO TREE - GEO TREE*/
GEOItem = pCtrl->GetNextItem(GEOItem, TVGN_FIRSTVISIBLE);
item_text = pCtrl->GetItemText(GEOItem);
First_Level_Item = pCtrl->GetNextItem(GEOItem, TVGN_CHILD);
item_text = pCtrl->GetItemText(First_Level_Item);
bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);
if ((nImage==0 || nImage==1) && First_Level_Item != NULL) GetCheckBox(First_Level_Item, CheckBoxOn);
while (pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT))
{
  First_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT);
  item_text = pCtrl->GetItemText(First_Level_Item);
  bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);

  Second_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_CHILD);
  item_text = pCtrl->GetItemText(Second_Level_Item);
  bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);

  while (pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT))
  {
    Second_Level_Item = pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT);
    item_text = pCtrl->GetItemText(Second_Level_Item);
    bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);
  }
}

/*MSH TREE - MSH TREE - MSH TREE*/
MSHItem = pCtrl->GetNextItem(GEOItem, TVGN_NEXT);
item_text = pCtrl->GetItemText(MSHItem);
First_Level_Item = pCtrl->GetNextItem(MSHItem, TVGN_CHILD);
item_text = pCtrl->GetItemText(First_Level_Item);
bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);
if ((nImage==0 || nImage==1) && First_Level_Item != NULL) GetCheckBox(First_Level_Item, CheckBoxOn);
while (pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT))
{
  First_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT);
  item_text = pCtrl->GetItemText(First_Level_Item);
  bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);

  Second_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_CHILD);
  item_text = pCtrl->GetItemText(Second_Level_Item);
  bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);

  while (pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT))
  {
    Second_Level_Item = pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT);
    item_text = pCtrl->GetItemText(Second_Level_Item);
    bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);
  }
}
/*FEM TREE - FEM TREE - FEM TREE*/
FEMItem = pCtrl->GetNextItem(MSHItem, TVGN_NEXT);
item_text = pCtrl->GetItemText(FEMItem);
First_Level_Item = pCtrl->GetNextItem(FEMItem, TVGN_CHILD);
item_text = pCtrl->GetItemText(First_Level_Item);
bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);
if ((nImage==0 || nImage==1) && First_Level_Item != NULL) GetCheckBox(First_Level_Item, CheckBoxOn);
while (pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT))
{
  First_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT);
  item_text = pCtrl->GetItemText(First_Level_Item);
  bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);

  Second_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_CHILD);
  item_text = pCtrl->GetItemText(Second_Level_Item);
  bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);

  while (pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT))
  {
    Second_Level_Item = pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT);
    item_text = pCtrl->GetItemText(Second_Level_Item);
    bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);
  }
}
/*MAT TREE - MAT TREE - MAT TREE*/
MATItem = pCtrl->GetNextItem(FEMItem, TVGN_NEXT);
item_text = pCtrl->GetItemText(MATItem);
First_Level_Item = pCtrl->GetNextItem(MATItem, TVGN_CHILD);
item_text = pCtrl->GetItemText(First_Level_Item);
bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);
if ((nImage==0 || nImage==1) && First_Level_Item != NULL) GetCheckBox(First_Level_Item, CheckBoxOn);
while (pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT))
{
  First_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_NEXT);
  item_text = pCtrl->GetItemText(First_Level_Item);
  bSuccess = pCtrl->GetItemImage(First_Level_Item, nImage, nSelectedImage);

  Second_Level_Item = pCtrl->GetNextItem(First_Level_Item, TVGN_CHILD);
  item_text = pCtrl->GetItemText(Second_Level_Item);
  bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);

  while (pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT))
  {
    Second_Level_Item = pCtrl->GetNextItem(Second_Level_Item, TVGN_NEXT);
    item_text = pCtrl->GetItemText(Second_Level_Item);
    bSuccess = pCtrl->GetItemImage(Second_Level_Item, nImage, nSelectedImage);
  }
}


}
