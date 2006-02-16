// GeoSysList.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysListView.h"
#include "msh_elements_rfi.h"
#include "msh_quality.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeoSysList

IMPLEMENT_DYNCREATE(CGeoSysList, CListView)

CGeoSysList::CGeoSysList()
{

}

CGeoSysList::~CGeoSysList()
{
}


BEGIN_MESSAGE_MAP(CGeoSysList, CListView)
	//{{AFX_MSG_MAP(CGeoSysList)
	ON_WM_CREATE()
	ON_COMMAND(ID_LIST_AREAQUALITY,OnAreaQualityList)
	ON_COMMAND(ID_LIST_ANGLEQUALITY,OnAngleQualityList)
	ON_COMMAND(ID_LIST_LENGTHQUALITY,OnLengthQualityList)
	ON_COMMAND(ID_LIST_TETRAVOLUMEQUALITY,OnTetraVolumeQualityList)
	ON_COMMAND(ID_LIST_TETRAANGLEQUALITY,OnTetraAngleQualityList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysList drawing

void CGeoSysList::OnDraw(CDC* pDC)
{
    pDC=pDC;//TK
    //CDocument* pDoc = GetDocument();
	// TODO: Add your specialized creation code here

	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysList diagnostics

#ifdef _DEBUG
void CGeoSysList::AssertValid() const
{
	CListView::AssertValid();
}

void CGeoSysList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeoSysList message handlers

void CGeoSysList::OnInitialUpdate() 
{
  CListView::OnInitialUpdate();
  // TODO: Add your specialized creation code here
  //FillList(sceneselect);

  // TODO: Add your specialized code here and/or call the base class
	
}
/**************************************************************************
GEOSYS-Methods for ListView
Task: Füllt die Tabelle auf
Programing:
03/2004 TK Splittered View with Tabs
**************************************************************************/
int CGeoSysList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CListView::OnCreate(lpCreateStruct) == -1)
  return -1;
  /*Code*/ 
  return 0;
}

BOOL CGeoSysList::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= (LVS_SHOWSELALWAYS | LVS_REPORT | LVS_EX_GRIDLINES |
               /*LVS_SINGLESEL | */LVS_NOSORTHEADER);
	
	return CListView::PreCreateWindow(cs);
}

void CGeoSysList::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CListView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


/**************************************************************************
GEOSYS-Methods for ListView
Task: Füllt die Tabelle auf
Programing:
03/2004 TK Splittered View with Tabs
**************************************************************************/
void CGeoSysList::FillList(int sceneselect) 
{
  long number_of_all_elements = 0;
  int i=0;
  double qvalue;
  char x[56];
  char y[56];
  vector<CMSHElements*> element_vector; 
  CListCtrl& lc = GetListCtrl();
  lc.DeleteAllItems();
  lc.DeleteColumn(0);
  lc.DeleteColumn(0);
  element_vector.clear();

  switch (sceneselect) {   
	case 1 :
		element_vector = MSHGetElementVector();
		number_of_all_elements = (int)element_vector.size();		 
		// insert columns
		lc.InsertColumn(0,"X (ID)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (Area Q-Factor)",LVCFMT_LEFT,100);
		lc.InsertColumn(2," ! ",LVCFMT_LEFT,50);
		// insert rows
		for (i=0;i<number_of_all_elements;i++)
		{
		sprintf(x,"%d",i);
		sprintf(y,"%g",element_vector[i]->area_quality);
		lc.InsertItem(i,x);
		lc.SetItemText(i,1,y);
		qvalue = element_vector[i]->area_quality;
		if (qvalue < 0.5) lc.SetItemText(i,2,"low");
		}
   
	break;
	case 2 :  
		element_vector = MSHGetElementVector();
		number_of_all_elements = (int)element_vector.size();	
		// insert columns
		lc.InsertColumn(0,"X (ID)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (Angle Q-Factor)",LVCFMT_LEFT,100);
		lc.InsertColumn(2,"!",LVCFMT_LEFT,50);
		// insert rows
		for (i=0;i<number_of_all_elements;i++)
		{
		sprintf(x,"%d",i);
		sprintf(y,"%g",element_vector[i]->angle_quality);
		lc.InsertItem(i,x);
		lc.SetItemText(i,1,y);
		qvalue = element_vector[i]->angle_quality;
		if (qvalue < 0.5) lc.SetItemText(i,2,"low");

		}		
	break;
		case 3 :  
		element_vector = MSHGetElementVector();
		number_of_all_elements = (int)element_vector.size();	
		// insert columns
		lc.InsertColumn(0,"X (ID)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (Length Q-Factor)",LVCFMT_LEFT,100);
		lc.InsertColumn(2,"!",LVCFMT_LEFT,50);
		// insert rows
		for (i=0;i<number_of_all_elements;i++)
		{
		sprintf(x,"%d",i);
		sprintf(y,"%g",element_vector[i]->length_quality);
		lc.InsertItem(i,x);
		lc.SetItemText(i,1,y);
		qvalue = element_vector[i]->length_quality;
		if (qvalue < 0.5) lc.SetItemText(i,2,"low");
		}		
	break;
		case 10 :  
		element_vector = MSHGetElementVector();
		number_of_all_elements = (int)element_vector.size();	
		// insert columns
		lc.InsertColumn(0,"X (ID)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (Volume Q-Factor)",LVCFMT_LEFT,100);
		lc.InsertColumn(2,"!",LVCFMT_LEFT,50);
		// insert rows
		for (i=0;i<number_of_all_elements;i++)
		{
		sprintf(x,"%d",i);
		sprintf(y,"%g",element_vector[i]->volume_quality);
		lc.InsertItem(i,x);
		lc.SetItemText(i,1,y);
		qvalue = element_vector[i]->volume_quality;
		if (qvalue < 0.5) lc.SetItemText(i,2,"low");

		}		
	break;
		case 11 :  
		element_vector = MSHGetElementVector();
		number_of_all_elements = (int)element_vector.size();	
		// insert columns
		lc.InsertColumn(0,"X (ID)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (Angle Q-Factor)",LVCFMT_LEFT,100);
		lc.InsertColumn(2,"!",LVCFMT_LEFT,50);
		// insert rows
		for (i=0;i<number_of_all_elements;i++)
		{
		sprintf(x,"%d",i);
		sprintf(y,"%g",element_vector[i]->angle_quality);
		lc.InsertItem(i,x);
		lc.SetItemText(i,1,y);
		qvalue = element_vector[i]->angle_quality;
		if (qvalue < 0.5) lc.SetItemText(i,2,"low");

		}		
	break;

	default:
		lc.InsertColumn(0,"X (no values)",LVCFMT_LEFT,50);
		lc.InsertColumn(1,"Y (no values)",LVCFMT_LEFT,150);
		lc.InsertColumn(2,"!",LVCFMT_LEFT,4);
		break;
  }
 
}


void CGeoSysList::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    pHint=pHint;//TK 
    pSender=pSender;//TK

  	if(lHint==0)
	{
		FillList(sceneselect);
	
		this->Invalidate();
	}
}

void CGeoSysList::OnAreaQualityList() 
{
	sceneselect=1;
	FillList(sceneselect);
    Invalidate(TRUE);
}

void CGeoSysList::OnAngleQualityList() 
{
	sceneselect=2;
	FillList(sceneselect);
    Invalidate(TRUE);
}

void CGeoSysList::OnLengthQualityList() 
{
	sceneselect=3;
	FillList(sceneselect);
    Invalidate(TRUE);
}

void CGeoSysList::OnTetraVolumeQualityList() 
{
	sceneselect=10;
	FillList(sceneselect);
    Invalidate(TRUE);
}

void CGeoSysList::OnTetraAngleQualityList() 
{
	sceneselect=11;
	FillList(sceneselect);
    Invalidate(TRUE);
}
