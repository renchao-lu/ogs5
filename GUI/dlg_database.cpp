/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/
#include "stdafx.h"
#include "GeoSys.h"
#include "dlg_database.h"
#include ".\dlg_database.h"
#include "MainFrm.h"
#include "gs_project.h"
// CDialogDatabase dialog

IMPLEMENT_DYNAMIC(CDialogDatabase, CDialog)

CDialogDatabase::CDialogDatabase(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogDatabase::IDD, pParent)
	, m_db_Filename(_T(""))
	
	, m_newtbname_db(_T(""))
{

}

CDialogDatabase::~CDialogDatabase()
{
}

void CDialogDatabase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DB_EDIT, m_db_Filename);

	DDX_Control(pDX, IDC_LIST_GEO, m_TableList);
	DDX_Control(pDX, IDC_LIST_ATT, m_AttributeList);
	DDX_Control(pDX, IDC_COMBO_PNT, m_db_pnt);
	DDX_Control(pDX, IDC_COMBO_PLY, m_db_ply);
	DDX_Control(pDX, IDC_COMBO_SFC, m_db_sfc);
	DDX_Control(pDX, IDC_LIST_DATA, m_listctrlattdata);
	DDX_Text(pDX, IDC_EDIT_NEWTB, m_newtbname_db);
}


BEGIN_MESSAGE_MAP(CDialogDatabase, CDialog)
	ON_BN_CLICKED(IDC_OPEN_DB, &CDialogDatabase::OnBnClickedOpenDb)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CDialogDatabase::OnBnClickedButtonImport)
	ON_LBN_SELCHANGE(IDC_LIST_GEO, &CDialogDatabase::OnLbnSelchangeList)
	ON_BN_CLICKED(IDC_BUTTON_ATT, &CDialogDatabase::OnBnClickedButtonAttribute)
	ON_BN_CLICKED(IDC_BUTTON_ELE, &CDialogDatabase::OnBnClickedButtonEle)
	ON_BN_CLICKED(IDOK, &CDialogDatabase::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_EXPDB, &CDialogDatabase::OnBnClickedButtonExpdb)
	ON_BN_CLICKED(IDC_BUTTON_MAT, &CDialogDatabase::OnBnClickedButtonMat)
END_MESSAGE_MAP()


// CDialogDatabase message handlers
/**************************************************************************
GeoSys GUI - Object: 
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/
BOOL CDialogDatabase::OnInitDialog() 
{
    CDialog::OnInitDialog();
    CRect           rect;
    CString         strItem1= _T("Field1");
	CString         strItem2= _T("Field2");
    CString         strItem3= _T("Field3");
    CString         strItem4= _T("Field4");
    CString         strItem5= _T("Field5");
    CString         strItem6= _T("Field6");
    m_listctrlattdata.GetWindowRect(&rect); 
    m_listctrlattdata.SetTextBkColor(RGB(153,153,255));
    m_listctrlattdata.SetTextColor(RGB(0,0,255));
    //pause the mouse with highlight or you can select it. so this will be the hot item.
    m_listctrlattdata.EnableTrackingToolTips();
    m_listctrlattdata.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT );
	m_listctrlattdata.InsertColumn(0, strItem1, LVCFMT_LEFT,
		rect.Width() * 1/6, 0);
	m_listctrlattdata.InsertColumn(1, strItem2, LVCFMT_LEFT,
		rect.Width() * 1/6, 1);
    m_listctrlattdata.InsertColumn(2, strItem3, LVCFMT_LEFT,
		rect.Width() * 1/6, 2);
    m_listctrlattdata.InsertColumn(3, strItem4, LVCFMT_LEFT,
		rect.Width() * 1/6, 3);
    m_listctrlattdata.InsertColumn(4, strItem5, LVCFMT_LEFT,
		rect.Width() * 1/6, 4);
    m_listctrlattdata.InsertColumn(5, strItem6, LVCFMT_LEFT,
        rect.Width() * 1/6, 5);
  

  return TRUE; // return TRUE unless you set the focus to a control	              // EXCEPTION: OCX Property Pages should return FALSE
}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing: open personal geodatabase
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedOpenDb()
{
    CFileDialog fileDlg(TRUE,"mdb",NULL,OFN_ENABLESIZING,"Geodatabase Files (*.mdb)|*.mdb| ArcView Files (*.shp)|*.shp| All Files (*.*)|*.*||", this );
    if (fileDlg.DoModal()==IDOK) {
    // Test
    CString SHPBaseType = fileDlg.GetFileName(); // base_type
    if(SHPBaseType.IsEmpty())
      return; 
    // Determine path plus base name
    CString SHPPathBaseType = fileDlg.GetPathName();
    int pos = SHPPathBaseType.ReverseFind('.');
    if(pos<1)
      return; 
    m_strDBPathBase = SHPPathBaseType.Left(pos);
	//file type cast
	char* shp_name = NULL;
	int datlen = m_strDBPathBase.GetLength();
    shp_name = (char *) Malloc(datlen);
	strcpy(shp_name,m_strDBPathBase);
	CString m_strFileNameExtension = SHPPathBaseType.Right(4);
	m_db_Filename = m_strDBPathBase + m_strFileNameExtension;
    UpdateData(FALSE);  
    }
    OGRRegisterAll();
    OGRFeatureDefn * featureDef;

    poDS = OGRSFDriverRegistrar::Open( m_db_Filename, FALSE );

    if( poDS == NULL )
    {
        AfxMessageBox("open failed.\n");
        exit;
    }
    OGRLayer  *poLayer;
    int layerNu = poDS->GetLayerCount();
	for(int i=0;i<layerNu;i++){
		poLayer = poDS->GetLayer(i);
		featureDef = poLayer->GetLayerDefn();
		m_TableList.AddString(featureDef->GetName());
    }
	UpdateData(FALSE);

  }
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:Import data from geodatabase
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedButtonImport()
{
  OGRFeature *poFeature;
  OGRPoint *poPoint;
  OGRLineString *poPolyline;
  OGRPolygon *poPolygon;
  int NumPns;
  CString m_strCount;
  int no_polylines = -1;
  int no_points = -1;
  int pnt_number = 0;
  int j;
  CGLPolyline* m_ply = NULL;
  CGLPoint m_pnt;
  CGLPoint* m_point = NULL;
  CGLPoint *m_pnt_exist = NULL;
  CGLPoint *m_pnt_new = NULL;
  long number_of_points = 0;

  poLayer->ResetReading();
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {

        OGRGeometry *poGeometry;
        poGeometry = poFeature->GetGeometryRef();
		if( poGeometry != NULL)
		switch (wkbFlatten(poGeometry->getGeometryType()))
        {
         case wkbPoint: // POINT
         poPoint = (OGRPoint *) poGeometry;
	     m_point = new CGLPoint();
         m_point->x = poPoint->getX();
         m_point->y = poPoint->getY();
		 m_point->z = poPoint->getZ();
  
         pnt_number = m_point->IsPointExist();
          if(pnt_number<0){
	        number_of_points = (long)gli_points_vector.size();
		    m_point->id = number_of_points;
            no_points = (int)gli_points_vector.size();
            gli_points_vector.push_back(m_point);
			//no_points++;
		    m_strGHDBPoint = "POINT_";
            m_strCount.Format("%i",no_points);
            m_strGHDBPoint += m_strCount;
            m_db_pnt.AddString(m_strGHDBPoint);
            m_db_pnt.SetCurSel(0);
           }
		
         break;
         case wkbLineString: // POLYLINE
         poPolyline = (OGRLineString *) poGeometry;
		 NumPns = poPolyline->getNumPoints();
		 no_polylines = (int)polyline_vector.size();
         //no_polylines++;
         m_strGHDBPolyline = "POLYLINE_";
         m_strCount.Format("%i",no_polylines);
         m_strGHDBPolyline += m_strCount;
         m_db_ply.AddString(m_strGHDBPolyline);
         m_db_ply.SetCurSel(0);
         m_ply = GEOGetPLYByName((string)m_strGHDBPolyline);
         if(m_ply)
          GEORemovePLY(m_ply);
       
         m_ply = new CGLPolyline((string)m_strGHDBPolyline);
		
		 for(j=0;j<=NumPns-1;j++){
		  m_pnt.x = poPolyline->getX(j);
          m_pnt.y = poPolyline->getY(j);
		  m_pnt.z = poPolyline->getZ(j);
          m_pnt_exist = m_pnt.Exist();
          if(m_pnt_exist){
            m_ply->point_vector.push_back(m_pnt_exist);
          }
          else{
		    number_of_points = (long)gli_points_vector.size();//CC 2008
            m_pnt_new = new CGLPoint();
            m_pnt_new->x = m_pnt.x;
            m_pnt_new->y = m_pnt.y;
            m_pnt_new->z = m_pnt.z;
			m_pnt_new->id = number_of_points;
			no_points = (int)gli_points_vector.size();
            gli_points_vector.push_back(m_pnt_new);
          //  no_points = gli_point_vector.size();
			//no_points++;
		    m_strGHDBPoint = "POINT_";
            m_strCount.Format("%i",no_points);
            m_strGHDBPoint += m_strCount;
            m_db_pnt.AddString(m_strGHDBPoint);
            m_db_pnt.SetCurSel(0);
            m_ply->point_vector.push_back(m_pnt_new);
          }//end of for
         }
         polyline_vector.push_back(m_ply);
         break;
         case wkbPolygon: // POLYGON
         poPolygon = (OGRPolygon *) poGeometry;		
		 poPolyline = poPolygon->getExteriorRing();
		//poPolygon->PointOnSurface(OGRPoint *point);
		 NumPns = poPolyline->getNumPoints();
		 //-------------------------------------------------------
         //no_polylines++;
		 no_polylines = (int)polyline_vector.size();
         m_strGHDBPolyline = "POLYLINE_";
         m_strCount.Format("%i",no_polylines);
         m_strGHDBPolyline += m_strCount;
         m_db_ply.AddString(m_strGHDBPolyline);
         m_db_ply.SetCurSel(0);
         m_ply = GEOGetPLYByName((string)m_strGHDBPolyline);
         if(m_ply)
          GEORemovePLY(m_ply);
       
         m_ply = new CGLPolyline((string)m_strGHDBPolyline);
		
	    	for(j=0;j<=NumPns-1;j++){
		  m_pnt.x = poPolyline->getX(j);
          m_pnt.y = poPolyline->getY(j);
		  m_pnt.z = poPolyline->getZ(j);
          m_pnt_exist = m_pnt.Exist();
          if(m_pnt_exist){
            m_ply->point_vector.push_back(m_pnt_exist);
          }
          else{
		    number_of_points = (long)gli_points_vector.size();//CC 2008
            m_pnt_new = new CGLPoint();
            m_pnt_new->x = m_pnt.x;
            m_pnt_new->y = m_pnt.y;
            m_pnt_new->z = m_pnt.z;
			m_pnt_new->id = number_of_points;
            gli_points_vector.push_back(m_pnt_new);
			no_points++;
		    m_strGHDBPoint = "POINT_";
            m_strCount.Format("%i",no_points);
            m_strGHDBPoint += m_strCount;
            m_db_pnt.AddString(m_strGHDBPoint);
            m_db_pnt.SetCurSel(0);
            m_ply->point_vector.push_back(m_pnt_new);
          }//end of for
         }
         polyline_vector.push_back(m_ply);
		 //-----------------------------------------------------------
		 //poPolyline->
         break;
         default: AfxMessageBox("undeclared GEO type!",MB_OK,0);
         break;
	     }

     OGRFeature::DestroyFeature( poFeature );
    }
  GSPAddMember((string)m_strGSPFileBase + ".gli");
  //----------------------------------------------------------------------
  // Min/Max //OK
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  GEOCalcPointMinMaxCoordinates();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
  {
    theApp->g_graphics_modeless_dlg->m_dXmin = pnt_x_min;
    theApp->g_graphics_modeless_dlg->m_dXmax = pnt_x_max;
    theApp->g_graphics_modeless_dlg->m_dYmin = pnt_y_min;
    theApp->g_graphics_modeless_dlg->m_dYmax = pnt_y_max;
    // Shrink a bit
    theApp->g_graphics_modeless_dlg->m_dXmin -= 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dXmax += 0.05*(pnt_x_max-pnt_x_min);
    theApp->g_graphics_modeless_dlg->m_dYmin -= 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->m_dYmax += 0.05*(pnt_y_max-pnt_y_min);
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL)
  {
    if((int)polyline_vector.size()>0){
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
      theApp->g_graphics_modeless_dlg->m_iDisplayPLY = 1;
      theApp->g_graphics_modeless_dlg->UpdateData(FALSE);
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->OnDrawFromToolbar();
    }
  }
	//get surface
  CGLPolyline* m_db_poly = NULL;
  Surface* m_sfc = NULL;
  for(int i=0;i<(int)polyline_vector.size();i++){
    m_db_poly = polyline_vector[i];
    m_sfc = GEOGetSFCByName(m_db_poly->name);
    if(m_sfc){
      //continue;
      GEORemoveSFC(m_sfc);
    }
    m_sfc = new Surface();
    m_sfc->name = m_db_poly->name;
	m_db_sfc.AddString(m_sfc->name.data());
    m_db_sfc.SetCurSel(0);
    m_sfc->polyline_of_surface_vector.push_back(m_db_poly);
    surface_vector.push_back(m_sfc);
    GEOSurfaceTopology(); //OKSB
    m_sfc->PolygonPointVector();
  }
  OnInitDialog();
  //----------------------------------------------------------------------
//  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    if((int)surface_vector.size()>0){
      theApp->g_graphics_modeless_dlg->m_iDisplaySUF = 1;
      theApp->g_graphics_modeless_dlg->UpdateData(FALSE);
      CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
      mainframe->OnDrawFromToolbar();
    }
  }

   // OGRDataSource::DestroyDataSource( poDS );

}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnLbnSelchangeList()
{
	  //  AfxMessageBox("select another feature class.\n");
 m_AttributeList.ResetContent();
 index = m_TableList.GetCurSel();
 m_TableList.GetText(index,featurename);

//OGRFeature *poFeature;
 poLayer = poDS->GetLayerByName(featurename);
 OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		
 int a = poFDefn->GetFieldCount();
 int iField;
 for( iField = 0; iField < a; iField++ )
    {
     OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
     CString ab = poFieldDefn->GetNameRef();
     CString ad;
	 if( poFieldDefn->GetType() == OFTInteger )
       ad = "integer";		
     else if( poFieldDefn->GetType() == OFTReal )
       ad = "double";
	 else if( poFieldDefn->GetType() == OFTString )
	   ad = "string";
     else
       ad = "string";
 m_AttributeList.AddString(ab + "    ||     " + ad);
    }
}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:Show the attribute data
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedButtonAttribute()  
{ 
  m_listctrlattdata.DeleteAllItems();
 //m_attribute_value.ResetContent();
  int iNumSel = m_AttributeList.GetSelCount(); // get number of selected items
  LPINT lpiSelItems = new int[iNumSel]; // allocate memory for all of them
  m_AttributeList.GetSelItems(iNumSel, lpiSelItems);
  OGRFeature *poFeature;
  OGRFeatureDefn *poFDefn;
  CString sout1,sout2,sout3,sout4;
  long listip = 0;
  char sID[1024];
  //CString sID;
  poFDefn = poLayer->GetLayerDefn();
  poLayer->ResetReading();
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
	sprintf(sID,"%d",listip);
	lvitem.pszText=sID;
    lvitem.iItem=listip;//size of list
    lvitem.iSubItem=0;
	m_listctrlattdata.InsertItem(&lvitem);
	   for( int nCount = 0; nCount<iNumSel; nCount++ )
        {
		int iField = lpiSelItems[nCount];
         OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
		 
         if( poFieldDefn->GetType() == OFTInteger )
		   
		 {sout1.Format("%d", poFeature->GetFieldAsInteger( iField ));
		 m_listctrlattdata.SetItemText(listip,nCount+1,sout1);}
		 else if( poFieldDefn->GetType() == OFTReal ){
            sout2.Format("%f", poFeature->GetFieldAsDouble( iField ));
			m_listctrlattdata.SetItemText(listip,nCount+1,sout2);}
		 else if( poFieldDefn->GetType() == OFTString ){
            sout3 = poFeature->GetFieldAsString(iField);
			m_listctrlattdata.SetItemText(listip,nCount+1,sout3);}
		 else{
            sout4 = poFeature->GetFieldAsString(iField);
			 m_listctrlattdata.SetItemText(listip,nCount,sout4);
			}
         }
   ++listip;
   OGRFeature::DestroyFeature( poFeature );
  }
  delete [] lpiSelItems;
}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing: generate line elements based on the soil map
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedButtonEle()
{
  GEOSMessageHandler notice;
  GEOSMessageHandler err;
  initGEOS(notice, err);
  int iNumSel = m_AttributeList.GetSelCount(); // get number of selected items
  LPINT lpiSelItems = new int[iNumSel]; // allocate memory for all of them
  m_AttributeList.GetSelItems(iNumSel, lpiSelItems);
  OGRFeature *poFeature;
  OGRPolygon *poPolygon;
  OGRPoint *poPoint;
  OGRFeatureDefn *poFDefn;
  CFEMesh* m_msh = NULL;
  CFEMesh* m_msh_lin = NULL;
  CNode* m_node = NULL;
  CElem* m_ele = NULL;
  CString matName;
  int p;
  int patchIndex = -1;
  int indexAcount = -1;
  int SoilThickness;
  double soilThicknessSeg;
  OGRBoolean tl;
  double ts;
  //double tolerance = 1e-1;
  double tolerance = 0.2;
  poFDefn = poLayer->GetLayerDefn();
  poLayer->ResetReading();
  for (int i = 0; i<fem_msh_vector.size();i++)
     {
       m_msh = new CFEMesh();
	   m_msh = fem_msh_vector[i];
	   if (m_msh->ele_type == 4)//tri
		 {
	     m_msh_lin = new CFEMesh();
		 m_msh_lin->ele_type = 1;
		 m_msh_lin->pcs_name = "RICHARDS_FLOW";
		 for(int j=0;j<(long)m_msh->nod_vector.size();j++)
		 //for(int j=200;j<300;j++)
            {  
			tl=0;
			ts= 100.0;
			indexAcount = -1;
			poLayer->ResetReading();
			poPoint = new OGRPoint();
            poPoint->setX(m_msh->nod_vector[j]->X());
            poPoint->setY(m_msh->nod_vector[j]->Y()); 
			//poPoint->setX(350400.3436);
            //poPoint->setY(2900230.0001); 
			poPoint->setZ(0.0);
		  
			while( (poFeature = poLayer->GetNextFeature()) != NULL & tl == 0)
            {
    
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();
		    //get thickness
		    for( int nCount = 0; nCount<iNumSel; nCount++ )
            {
		     int iFieldth = lpiSelItems[nCount];
             OGRFieldDefn *poFieldDefn1 = poFDefn->GetFieldDefn( iFieldth );
		       if( poFieldDefn1->GetType() == OFTInteger )
		        {
                 SoilThickness = poFeature->GetFieldAsInteger( iFieldth );
			     soilThicknessSeg = (double)SoilThickness/100;
		        }
			 //end of get thickness
             } 
		//end of get thickness
		//----------------------------------------------
		    if( poGeometry != NULL)
		    switch (wkbFlatten(poGeometry->getGeometryType()))
             {
              case wkbPolygon: // POINT
              poPolygon = (OGRPolygon *) poGeometry;
              break;
		      default: AfxMessageBox("not a polygon type!",MB_OK,0);
              break;
	          }
           
           tl = poPoint->Within(poPolygon);
		   ts = poPolygon->Distance(poPoint);
		   if (ts<tolerance)
		   tl=1;
           indexAcount++;
          }//end of while
     //here add new node and new element     
			for (int n = 0;n<=10;n++)
			{
			double a = m_msh->nod_vector[j]->X();
            double b = m_msh->nod_vector[j]->Y();
            double c = m_msh->nod_vector[j]->Z() - (double)soilThicknessSeg*n/10;
			m_node= new CNode((int)m_msh_lin->nod_vector.size(),a,b,c);
			m_node->SetIndex((long)m_msh_lin->nod_vector.size());
            m_msh_lin->nod_vector.push_back(m_node);
		    }
			//mat group
			if (indexAcount != -1)
			{
			 matName = mat_name[indexAcount];
			 int pos = matName.ReverseFind('S');
             int lengthstring = matName.GetLength();
             int pos2 = lengthstring - pos -1;
             matName = matName.Right(pos2);
		     patchIndex = atoi(matName)-1;
			}
			else
				patchIndex = -1;
			//
			for (int m = 0;m<10;m++){
            m_ele = new CElem();
			m_ele->nodes_index.resize(2);
			m_ele->SetNodesNumber(2);
			m_ele->nodes_index[0]=11*j+m;
			m_ele->nodes_index[1]=11*j+m+1;
			//m_ele->geo_type = 1;
			m_ele->SetElementType(1);//line  		      
			m_ele->SetPatchIndex(patchIndex);
            m_msh_lin->ele_vector.push_back(m_ele);
			}
       }//end of for nod_vector
		 m_msh_lin->InitialNodesNumber();
		 //m_msh_lin->NodesNumber_Linear = (long)m_msh_lin->nod_vector.size();
		 fem_msh_vector.push_back(m_msh_lin);
      }//end of if ele_type 4
     }//end of fem_vector
  AfxMessageBox("Line elements are generated",MB_OK,0);  
  delete [] lpiSelItems;
  finishGEOS();
}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedOk()
{
	OGRDataSource::DestroyDataSource( poDS );
	OnOK();
}

void CDialogDatabase::OnBnClickedButtonExpdb()
{
	// TODO: Add your control notification handler code here
}
/**************************************************************************
GeoSys GUI - Object: GHDB database interface Dialog
Task: 
Programing:
01/2009 CC Implementation
**************************************************************************/
void CDialogDatabase::OnBnClickedButtonMat()
{
  int iNumSel = m_AttributeList.GetSelCount(); // get number of selected items
  LPINT lpiSelItems = new int[iNumSel]; // allocate memory for all of them
  m_AttributeList.GetSelItems(iNumSel, lpiSelItems);

  CString scout;
  OGRFeature *poFeature;
  OGRFeatureDefn *poFDefn;
  poFDefn = poLayer->GetLayerDefn();
  poLayer->ResetReading();
  while( (poFeature = poLayer->GetNextFeature()) != NULL)
            {
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();
		    //get thickness
		    for( int nCount = 0; nCount<iNumSel; nCount++ )
            {
		     int iFieldth = lpiSelItems[nCount];
             OGRFieldDefn *poFieldDefn1 = poFDefn->GetFieldDefn( iFieldth );
		        if (poFieldDefn1->GetType() == OFTString )
			   {
                scout =   poFeature->GetFieldAsString(iFieldth);
				mat_name.push_back(scout);
			   }
			 //end of get thickness
             } 
  }
AfxMessageBox("MAT are generated",MB_OK,0);  
}
