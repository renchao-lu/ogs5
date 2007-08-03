// GSPropertyRightResults.cpp : implementation file
//
#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "GSPropertyRightResults.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
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

#include "DlgIsoListValue.h"
#include "DlgIsoFrameWidth.h"
#include ".\gspropertyrightresults.h"
#include "IsoColArrangement.h"

// CGSPropertyRightResults dialog

IMPLEMENT_DYNAMIC(CGSPropertyRightResults, CViewPropertyPage)
CGSPropertyRightResults::CGSPropertyRightResults()
	: CViewPropertyPage(CGSPropertyRightResults::IDD)
{
    m_pcs_min_r= 0.0;
    m_pcs_max_r= 0.0;
    GetPcsMinmax();
    //HS, Iso function
	Iso_Form = 0;
	Iso_Type = 0;
	If_Switch_Off_ContourPlot = false;
	Iso_If_Show_Iso = false;
	Iso_Step_Value = 0;
	Iso_Min_Value = 0;
	Iso_Max_Value = 0;
	Iso_Num = 10;
	for (int i=0;i<1000;i++)
	{
	IsoColor[i]= RGB(255,255,255);
	IsoFrameColor[i]= RGB(255,255,255);
	IsoFrameWidth[i]= 1.0;
    }

}

CGSPropertyRightResults::~CGSPropertyRightResults()
{
}

void CGSPropertyRightResults::DoDataExchange(CDataExchange* pDX)
{
	CViewPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_PCS_MIN_EDIT, m_pcs_min_r);
    DDX_Text(pDX,IDC_PCS_MAX_EDIT, m_pcs_max_r);
    DDX_Control(pDX, IDC_SLIDER1, m_slider1);
    DDX_Control(pDX, IDC_SLIDER2, m_slider2);
 	DDX_Control(pDX, IDC_CHECK_If_Show_Iso, Control_Show_Iso);
	DDX_Control(pDX, IDC_CHECK3, Control_SwitchOff_ContourPlot);
	DDX_Control(pDX, IDC_LIST7, m_Iso_List_1);
	DDX_Text(pDX,IDC_EDIT_Num, Iso_Num);
    //m_pmodeless_r = NULL;


}


BEGIN_MESSAGE_MAP(CGSPropertyRightResults, CViewPropertyPage)
    ON_BN_CLICKED(IDC_GET_PCS_MINMAX_BUTTON3, OnBnClickedGetPcsMinmaxButton3)
    ON_BN_CLICKED(IDC_SET_PCS_MINMAX_BUTTON2, OnBnClickedSetPcsMinmaxButton2)
    ON_BN_CLICKED(IDC_VALUE_POINTS_BUTTON, OnBnClickedValuePointsButton)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnNMCustomdrawSlider1)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnNMCustomdrawSlider2)
	//ON_BN_CLICKED(IDC_ISOLINE_ISOSURFACE_BUTTON, OnBnClickedIsolineIsosurfaceButton)
	ON_BN_CLICKED(IDC_CHECK_If_Show_Iso, OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_Iso_Refresh, OnBnClickedIsoRefresh)
	ON_BN_CLICKED(IDC_Iso_AddListItem, OnBnClickedIsoAddlistitem)
	ON_BN_CLICKED(IDC_Iso_DelListItem, OnBnClickedIsoDellistitem)
	ON_BN_CLICKED(IDOK, OnBnClickedIsoApply)
	ON_NOTIFY( NM_CUSTOMDRAW, IDC_LIST7, OnCustomdrawMyIsoList )
	ON_NOTIFY( NM_CLICK, IDC_LIST7, OnClickMyIsoList )
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillFocusIsoValueEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST7, OnNMDblclkList6)
	ON_WM_KEYDOWN()
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickIsoListCtrl)

	ON_BN_CLICKED(IDC_Iso_Refresh2, OnBnClickedIsoRefresh2)
	ON_BN_CLICKED(IDC_Iso_AddListItem2, OnBnClickedIsoAddlistitem2)
	ON_BN_CLICKED(IDC_Iso_DelListItem2, OnBnClickedIsoDellistitem2)
	ON_BN_CLICKED(IDC_Iso_DeleteAll, OnBnClickedIsoDeleteall)
END_MESSAGE_MAP()


// CGSPropertyRightResults message handlers

BOOL CGSPropertyRightResults::OnInitDialog()
{

    CDialog::OnInitDialog();

     CRect rect;
     m_Iso_List_1.GetClientRect(&rect);
     int nColInterval = rect.Width()/10;
	 //CWnd *pParent = m_Iso_List_1.GetParent();
     m_Iso_List_1.InsertColumn(0, _T("#"), LVCFMT_LEFT, nColInterval*2);
     m_Iso_List_1.InsertColumn(1, _T("Value"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List_1.InsertColumn(2, _T("Iso_Col"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List_1.InsertColumn(3, _T("Frame_Col"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List_1.InsertColumn(4, _T("Width"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List_1.InsertColumn(5, _T("Line"), LVCFMT_LEFT, rect.Width()-8*nColInterval);
     m_Iso_List_1.ModifyStyle(0, LVS_REPORT);

     ListView_SetExtendedListViewStyle(::GetDlgItem(m_hWnd,IDC_LIST7),LVS_EX_FULLROWSELECT | 
        LVS_EX_GRIDLINES); 

     ::ShowWindow(::GetDlgItem(m_hWnd,IDC_EDIT1),SW_HIDE);


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CGSPropertyRightResults::OnBnClickedGetPcsMinmaxButton3()
{
 CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 GetPcsMinmax();
 UpdateData(FALSE);
 m_frame->m_something_changed = 1;
 //m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
 m_pDoc->UpdateAllViews(NULL);
 Invalidate(TRUE);
}

/**************************************************************************
GUILib-Method:
??/200? ?? Implementation
07/2007 OK PV vector
**************************************************************************/
void CGSPropertyRightResults::GetPcsMinmax()
{
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  CRFProcess* m_pcs = NULL;
  if(pcs_vector.size()==0)
    return;
  m_pcs = PCSGet((string)mainframe->m_pcs_name);
  if(!m_pcs)
  {
    AfxMessageBox("CGSPropertyRightResults::GetPcsMinmax() - no PCS data");
    return;
  }
  double value;
  m_pcs_min_r = 1.e+19;
  m_pcs_max_r = -1.e+19;
  int nidx = m_pcs->GetNodeValueIndex((string)mainframe->m_variable_name);
  for(long j=0;j<(long)m_pcs->nod_val_vector.size();j++)
  {
    value = m_pcs->GetNodeValue(j,nidx);
    if(value<m_pcs_min_r) m_pcs_min_r = value;
    if(value>m_pcs_max_r) m_pcs_max_r = value;
  }  
  mainframe->m_pcs_min = m_pcs_min_r;
  mainframe->m_pcs_max = m_pcs_max_r;
  mainframe->m_something_changed = 1;
}

void CGSPropertyRightResults::OnBnClickedSetPcsMinmaxButton2()
{
    UpdateData(TRUE);
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    m_frame->m_pcs_min = m_pcs_min_r;
    m_frame->m_pcs_max = m_pcs_max_r;
    m_frame->m_something_changed = 1;
    m_pDoc->UpdateAllViews(NULL);
    Invalidate(TRUE);
}

void CGSPropertyRightResults::OnBnClickedValuePointsButton()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    m_frame->move_distance = 0;
    m_frame->pcs_value_distort_factor = 0;
    if (m_frame->m_pcs_values_mesh != 1)
	{
      m_frame->m_pcs_values_mesh = 1;
      m_slider1.SetPos(10);
	  m_slider1.SetPos(0);
      m_slider2.SetPos(10);
	  m_slider2.SetPos(0);
	}
    else 
	{
	  m_frame->m_pcs_values_mesh = 0;
	  m_slider1.SetPos(-100);
      m_slider2.SetPos(-100);
	}
    m_frame->m_something_changed = 1;
	m_frame-> UpdateSpecificView("COGLView", m_pDoc);
    //Invalidate(TRUE);
}

void CGSPropertyRightResults::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    pNMCD=pNMCD;
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    m_slider1.SetRangeMin(-100);
    m_slider1.SetRangeMax(100);
    //int max = m_slider1.GetRangeMax();
    //int min = m_slider1.GetRangeMin(); 
    int pos = m_slider1.GetPos();
    if (pos != m_frame->move_distance)
    {
    m_frame->move_distance = pos;
    //START UpdateSpecificViews   
    m_frame->UpdateSpecificView("COGLView", m_pDoc);
    }
    *pResult = 0;
}

void CGSPropertyRightResults::OnNMCustomdrawSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	pNMCD=pNMCD;
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
    CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
    m_slider2.SetRangeMin(-100);
    m_slider2.SetRangeMax(100);
    //int max = m_slider2.GetRangeMax();
    //int min = m_slider2.GetRangeMin();
    int pos = m_slider2.GetPos();
    if (pos != m_frame->pcs_value_distort_factor)
    {
    m_frame->pcs_value_distort_factor = pos;
    //START UpdateSpecificViews   
    m_frame-> UpdateSpecificView("COGLView", m_pDoc);
    }
    *pResult = 0;
}


void CGSPropertyRightResults::OnBnClickedCheck2()
{
    OnBnClickedGetPcsMinmaxButton3();
	UpdateData(true);
	if (Control_Show_Iso.GetCheck() == BST_CHECKED)
	{
		Iso_If_Show_Iso = true;
	}
	else
	{
		Iso_If_Show_Iso = false;
	}
	OnBnClickedIsoRefresh2();
	UpdateData(false);
}

void CGSPropertyRightResults::OnBnClickedCheck3()
{
	UpdateData(true);
	if (Control_SwitchOff_ContourPlot.GetCheck() == BST_CHECKED)
	{
		If_Switch_Off_ContourPlot = true;
		UpdateData(false);
	}
	else
	{
		If_Switch_Off_ContourPlot = false;
	}
}
void CGSPropertyRightResults::InsertIsoListItem(double max, double min, int Num)
{
    m_Iso_List_1.DeleteAllItems();

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	for (int i = 0; i < Num; i++)
	{
		// Insert the first item
		lvi.mask =  LVIF_TEXT;
		strItem.Format(_T(" %i"), i);
	
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.InsertItem(&lvi);

		// Set subitem 1
		strItem.Format(_T("%f"), min + (max-min)/Num*((double)(i+1)));
		lvi.iSubItem =1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);

		// Set subitem 2
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);

		// Set subitem 3
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);

		// Set subitem 4
		strItem.Format(_T("%1.1f"), IsoFrameWidth[i]);
		lvi.iSubItem =4;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);


	}

}
void CGSPropertyRightResults::OnBnClickedIsoRefresh()
{
	UpdateData(true);
	CString str;

	GetPcsMinmax();
	Iso_Min_Value = m_pcs_min_r;
	Iso_Max_Value = m_pcs_max_r;
	Iso_Step_Value = (Iso_Max_Value-Iso_Min_Value)/Iso_Num;

	str.Format("%f",Iso_Min_Value);
	SetDlgItemText(IDC_STATIC_MinValue,str);
	str.Format("%f",Iso_Max_Value);
	SetDlgItemText(IDC_STATIC_Max_Value,str);
	InsertIsoListItem(Iso_Max_Value,Iso_Min_Value,Iso_Num);
}

void CGSPropertyRightResults::OnBnClickedIsoAddlistitem()
{
	int count;
	LVITEM lvi;
	CString strItem;
	count = m_Iso_List_1.GetItemCount();
		
	lvi.mask =  LVIF_TEXT;
	strItem.Format(_T(" %i"), count);

	lvi.iItem = count;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.InsertItem(&lvi);

	// Set subitem 1
	strItem.Format(_T("%f"), 0);
	lvi.iSubItem =1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 2
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =2;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 3
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =3;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 4
	strItem.Format(_T("%1.1f"), 1.0);
	lvi.iSubItem =4;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);
	
}

void CGSPropertyRightResults::OnBnClickedIsoDellistitem()
{
	int mark;
	mark = m_Iso_List_1.GetSelectionMark();
	m_Iso_List_1.DeleteItem(mark);
}

void CGSPropertyRightResults::OnBnClickedIsoApply()
{
    CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
	// transfer data
	m_frame->Iso_If_Show_Iso = Iso_If_Show_Iso;
	m_frame->Iso_If_SwitchOff_ContourPlot = If_Switch_Off_ContourPlot;
	m_frame->Iso_Type = Iso_Type;
	m_frame->Iso_Form = Iso_Form;
	m_frame->Iso_Max_Value = Iso_Max_Value;
	m_frame->Iso_Min_Value = Iso_Min_Value;
	m_frame->Iso_Step_Value = (Iso_Max_Value-Iso_Min_Value)/(Iso_Num+1);
	int count;
	count =  m_Iso_List_1.GetItemCount();
	if (count <= 1000)
	{
		m_frame->Iso_Count = count;
		for(int i=0;i<count;i++)
		{
			CString str;
			str = m_Iso_List_1.GetItemText( i , 1 );
			m_frame->IsoValueArray[i] = atof(str.GetBuffer());
			m_frame->IsoColorArray[i] = IsoColor[i];
			m_frame->IsoFrameColorArray[i] = IsoFrameColor[i];
			str = m_Iso_List_1.GetItemText( i, 4 );
			m_frame->IsoWidthArray[i] = atof(str.GetBuffer());
		}

        CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
        CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
        CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 	//here add the funciton to call the redaw of the view
		m_pDoc->UpdateAllViews(NULL);
	}
	else
	{
		AfxMessageBox("Can not handle more than 1000 Iso-units. Please Modify. ");
	}

}
void CGSPropertyRightResults::OnCustomdrawMyIsoList( NMHDR* pNMHDR, LRESULT* pResult )
{

//---This function is from "Neat Stuff to do in List Controls Using Custom Draw" by Michal Dunn
NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.

    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        *pResult = CDRF_NOTIFYITEMDRAW;
        }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        // This is the notification message for an item.  We'll request
        // notifications before each subitem's prepaint stage.

        *pResult = CDRF_NOTIFYSUBITEMDRAW;
        }
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
        {
        // This is the prepaint stage for a subitem. Here's where we set the
        // item's text and background colors. Our return value will tell 
        // Windows to draw the subitem itself, but it will use the new colors
        // we set here.
        // The text color will cycle through red, green, and light blue.
        // The background color will be light blue for column 0, red for
        // column 1, and black for column 2.
    
        COLORREF crText, crBkgnd;
		int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
        
        if ( 0 == pLVCD->iSubItem )
            {
            crText = RGB(0,0,0);
            crBkgnd = RGB(255,255,255);
            }
        else if ( 1 == pLVCD->iSubItem )
            {
            crText = RGB(0,0,0);
            crBkgnd = RGB(255,255,255);
            }
		else if ( 2 == pLVCD->iSubItem )
            {
            crText = RGB(255,255,255);
			crBkgnd = IsoColor[nItem];//this should be the color from the memory.
            }
		else if ( 3 == pLVCD->iSubItem )
            {
            crText = RGB(255,255,255);
			crBkgnd = IsoFrameColor[nItem];//this should be the color from the memory.
            }
        else
            {
            crText = RGB(0,0,0);
            crBkgnd = RGB(255,255,255);
            }

        // Store the colors back in the NMLVCUSTOMDRAW struct.
        pLVCD->clrText = crText;
        pLVCD->clrTextBk = crBkgnd;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
        }

}
void CGSPropertyRightResults::SetCell(HWND hWnd1, CString value, int nRow, int nCol)
{
    TCHAR     szString [256];
    wsprintf(szString,value ,0);

    //Fill the LVITEM structure with the 
    //values given as parameters.
    LVITEM lvItem;
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nRow;
    lvItem.pszText = szString;
    lvItem.iSubItem = nCol;
    if(nCol >0)
        //set the value of listItem
        ::SendMessage(hWnd1,LVM_SETITEM, 
            (WPARAM)0,(WPARAM)&lvItem);
    else
        //Insert the value into List
        ListView_InsertItem(hWnd1,&lvItem);

}
CString CGSPropertyRightResults::GetItemText(HWND hWnd, int nItem, int nSubItem) const
{
    LVITEM lvi;
    memset(&lvi, 0, sizeof(LVITEM));
    lvi.iSubItem = nSubItem;
    CString str;
    int nLen = 128;
    int nRes;
    do
    {
        nLen *= 2;
        lvi.cchTextMax = nLen;
        lvi.pszText = str.GetBufferSetLength(nLen);
        nRes  = (int)::SendMessage(hWnd, 
            LVM_GETITEMTEXT, (WPARAM)nItem,
            (LPARAM)&lvi);
    } while (nRes == nLen-1);
    str.ReleaseBuffer();
    return str;
}
void CGSPropertyRightResults::OnClickMyIsoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    Invalidate();
    HWND hWnd1 =  ::GetDlgItem (m_hWnd,IDC_LIST7);
    LPNMITEMACTIVATE temp = (LPNMITEMACTIVATE) pNMHDR;
    RECT rect;
    //get the row number
    nItem = temp->iItem;
    //get the column number
    nSubItem = temp->iSubItem;
    if(nSubItem == 0 || nSubItem == -1 || nItem == -1 || nSubItem == 2 || nSubItem == 3 )
        return ;
    //Retrieve the text of the selected subItem 
    //from the list
    CString str = GetItemText(hWnd1,nItem,nSubItem);

    RECT rect1,rect2;
    // this macro is used to retrieve the Rectanle 
    // of the selected SubItem
    ListView_GetSubItemRect(hWnd1,temp->iItem,temp->iSubItem,LVIR_BOUNDS,&rect);
    //Get the Rectange of the listControl
    ::GetWindowRect(temp->hdr.hwndFrom,&rect1);
    //Get the Rectange of the Dialog
	GetClientRect(&rect2);
	ClientToScreen(&rect2);

	int x = rect1.left - rect2.left;
	int y = rect1.top - rect2.top;

	if(nItem != -1)
	::SetWindowPos(::GetDlgItem(m_hWnd,IDC_EDIT1), HWND_TOP,
	x + rect.left + 6, y + rect.top + 2,
	rect.right - rect.left - 3, rect.bottom - rect.top - 1,
	NULL
	); 
    ::ShowWindow(::GetDlgItem(m_hWnd,IDC_EDIT1),SW_SHOW);
    ::SetFocus(::GetDlgItem(m_hWnd,IDC_EDIT1));
    //Draw a Rectangle around the SubItem
	//Draw a Rectangle around the SubItem
	HDC hDC = ::GetDC(temp->hdr.hwndFrom);
	::Rectangle(hDC,rect.left,rect.top-1,rect.right,rect.bottom);
	::ReleaseDC(temp->hdr.hwndFrom, hDC);
    //Set the listItem text in the EditBox
    ::SetWindowText(::GetDlgItem(m_hWnd,IDC_EDIT1),str);
    *pResult = 0;
}

void CGSPropertyRightResults::OnKillFocusIsoValueEdit()
{
    CWnd* pwndCtrl = GetFocus();
    // get the control ID which is presently having the focus
	//int ctrl_ID = pwndCtrl->GetDlgCtrlID();
	CString str;
	//get the text from the EditBox
	GetDlgItemText(IDC_EDIT1,str);
	//set the value in the listContorl with the specified Item & SubItem values
	SetCell(::GetDlgItem (m_hWnd,IDC_LIST7),str,nItem,nSubItem);
	::SendDlgItemMessage(m_hWnd,IDC_EDIT1,WM_KILLFOCUS,0,0);
	::ShowWindow(::GetDlgItem(m_hWnd,IDC_EDIT1),SW_HIDE);
}

void CGSPropertyRightResults::OnNMDblclkList6(NMHDR *pNMHDR, LRESULT *pResult)
{
    Invalidate();
    HWND hWnd1 =  ::GetDlgItem (m_hWnd,IDC_LIST7);
    LPNMITEMACTIVATE temp = (LPNMITEMACTIVATE) pNMHDR;
    
    //get the row number
    nItem = temp->iItem;
    //get the column number
    nSubItem = temp->iSubItem;
    if(nSubItem == 0 || nSubItem == -1 || nItem == -1 || nSubItem == 1 || nSubItem == 4 )
        return ;
    //change color
	if ( nSubItem == 2 )
	{
		CColorDialog dlg;
		if ( dlg.DoModal() == IDOK )
		{
			IsoColor[temp->iItem]=dlg.GetColor();//recieve color information and store in memory
			//*pResult = 0;
			//OnCustomdrawMyIsoList( pNMHDR, pResult );
			m_Iso_List_1.RedrawItems(temp->iItem,temp->iItem);
		}
	}
	
	if ( nSubItem == 3 )
	{
		CColorDialog dlg;
		if ( dlg.DoModal() == IDOK )
		{
			IsoFrameColor[temp->iItem]=dlg.GetColor();//recieve color information and store in memory
			//*pResult = 0;
			//OnCustomdrawMyIsoList( pNMHDR, pResult );
			m_Iso_List_1.RedrawItems(temp->iItem,temp->iItem);
		}
	}
	*pResult = 0;
}

void CGSPropertyRightResults::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	nFlags = nFlags;
	nRepCnt = nRepCnt;
	if(nChar==VK_RETURN){
		OnKillFocusIsoValueEdit();
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CGSPropertyRightResults::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
	OnKeyDown((UINT)pMsg->wParam,(UINT)pMsg->lParam,(UINT)pMsg->lParam);
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CGSPropertyRightResults::OnItemclickIsoListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	NMHEADER *pHdr = (NMHEADER*)pNMHDR;

	// Get a pointer to the header control and figure out what happened
	CHeaderCtrl *pHeader = m_Iso_List_1.GetHeaderCtrl();
	ASSERT(pHeader);
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	TCHAR szItem[256];
	hdi.pszText = szItem;
	hdi.cchTextMax = 256;
	pHeader->GetItem(pHdr->iItem, &hdi);
	/*
	CString str;
	str.Format(_T("(HDN_ITEMCLICK) User selected column: %d, Title: %s"), pHdr->iItem, hdi.pszText);
	m_cHeaderMsg.AppendString(str);*/
	switch(pHdr->iItem)
	{
		case 1:
			{
				CDlgIsoListValue dlg;
				dlg.m_max = Iso_Max_Value;
				dlg.m_min = Iso_Min_Value;
				dlg.m_step = Iso_Step_Value;
				if (dlg.DoModal() == IDOK )
				{
					Iso_Max_Value = dlg.m_max;
					Iso_Min_Value = dlg.m_min;
					Iso_Step_Value = dlg.m_step;
					InsertIsoListItem(Iso_Max_Value, Iso_Min_Value, Iso_Step_Value);
				}
			break;
			}
		case 2:
			{
				CIsoColArrangement dlg;
				if (dlg.DoModal() == IDOK)
				{
					if (dlg.UsingUniformCol == TRUE )
					{
						int count;
						count =  m_Iso_List_1.GetItemCount();
						for (int i=0;i < count ;i++)
						{
							IsoColor[i]=dlg.UniformCol;
						}
						m_Iso_List_1.RedrawItems(0,count);
					}
					else
					{
						double value_norm;
						double value;
						int count;
						count =  m_Iso_List_1.GetItemCount();
						HWND hWnd2 =  ::GetDlgItem (m_hWnd,IDC_LIST7);
						CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
						for (int i=0;i < count ;i++)
						{
							CString str = GetItemText(hWnd2,i,1);
							value = atof(str.GetBuffer());
							value_norm = (value-m_frame->m_pcs_min)/(m_frame->m_pcs_max - m_frame->m_pcs_min);
							IsoColor[i] = RGB((double)(Get_Red_Value(value_norm)*255),(double)(Get_Green_Value(value_norm)*255),(double)(Get_Blue_Value(value_norm)*255));
						}
						m_Iso_List_1.RedrawItems(0,count);
					}
				}
			break;
			}
		case 3:
			{
				CIsoColArrangement dlg;
				if (dlg.DoModal() == IDOK)
				{
					if (dlg.UsingUniformCol == TRUE )
					{
						int count;
						count =  m_Iso_List_1.GetItemCount();
						for (int i=0;i < count ;i++)
						{
							IsoFrameColor[i]=dlg.UniformCol;
						}
						m_Iso_List_1.RedrawItems(0,count);
					}
					else
					{
						double value_norm;
						double value;
						int count;
						count =  m_Iso_List_1.GetItemCount();
						HWND hWnd2 =  ::GetDlgItem (m_hWnd,IDC_LIST7);
						CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
						for (int i=0;i < count ;i++)
						{
							CString str = GetItemText(hWnd2,i,1);
							value = atof(str.GetBuffer());
							value_norm = (value-m_frame->m_pcs_min)/(m_frame->m_pcs_max - m_frame->m_pcs_min);
							IsoFrameColor[i] = RGB((double)(Get_Red_Value(value_norm)*255),(double)(Get_Green_Value(value_norm)*255),(double)(Get_Blue_Value(value_norm)*255));
						}
						m_Iso_List_1.RedrawItems(0,count);
					}
				}
			break;
			}
		case 4:
			{
				CDlgIsoFrameWidth dlg;
				if ( dlg.DoModal() == IDOK )
				{
					int count;
					count =  m_Iso_List_1.GetItemCount();
					for (int i=0;i < count ;i++)
					{
						IsoFrameWidth[i]=dlg.Iso_Frame_Width;//here have to change
					}
					InsertIsoListItem(Iso_Max_Value, Iso_Min_Value, Iso_Step_Value);
				}
			break;
			}

	}
	
	*pResult = 0;
}
void CGSPropertyRightResults::InsertIsoListItem(double max, double min, double step)
{
    m_Iso_List_1.DeleteAllItems();
	int Num;
	if (step == 0)step=0.1;
	Num = (int)((max-min)/step)+1;

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	for (int i = 0; i < Num; i++)
	{
		// Insert the first item
		lvi.mask =  LVIF_TEXT;
		strItem.Format(_T(" %i"), i);
	
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.InsertItem(&lvi);

		// Set subitem 1
		strItem.Format(_T("%f"), min+i*step);
		lvi.iSubItem =1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);

		// Set subitem 2
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);

		// Set subitem 3
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);		
		
		// Set subitem 4
		strItem.Format(_T("%1.1f"), IsoFrameWidth[i]);
		lvi.iSubItem =4;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List_1.SetItem(&lvi);
	}
}


void CGSPropertyRightResults::OnBnClickedIsoRefresh2()
{
	UpdateData(true);
	CString str;

	GetPcsMinmax();
	Iso_Min_Value = m_pcs_min_r;
	Iso_Max_Value = m_pcs_max_r;
	Iso_Step_Value = (Iso_Max_Value-Iso_Min_Value)/Iso_Num;

	str.Format("%f",Iso_Min_Value);
	SetDlgItemText(IDC_STATIC_MinValue2,str);
	str.Format("%f",Iso_Max_Value);
	SetDlgItemText(IDC_STATIC_Max_Value2,str);
	InsertIsoListItem(Iso_Max_Value,Iso_Min_Value,Iso_Num);
}

void CGSPropertyRightResults::OnBnClickedIsoAddlistitem2()
{
	int count;
	LVITEM lvi;
	CString strItem;
	count = m_Iso_List_1.GetItemCount();
		
	lvi.mask =  LVIF_TEXT;
	strItem.Format(_T(" %i"), count);

	lvi.iItem = count;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.InsertItem(&lvi);

	// Set subitem 1
	strItem.Format(_T("%f"), 0);
	lvi.iSubItem =1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 2
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =2;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 3
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =3;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);

	// Set subitem 4
	strItem.Format(_T("%1.1f"), 1.0);
	lvi.iSubItem =4;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List_1.SetItem(&lvi);}

void CGSPropertyRightResults::OnBnClickedIsoDellistitem2()
{
	int mark;
	mark = m_Iso_List_1.GetSelectionMark();
	m_Iso_List_1.DeleteItem(mark);
}

void CGSPropertyRightResults::OnBnClickedIsoDeleteall()
{
    m_Iso_List_1.DeleteAllItems();
}
double CGSPropertyRightResults::Get_Red_Value(double value_norm) 
{
   	double Red=0.0;
	if (value_norm<0.25)                    Red   =0.0;
	if (value_norm>=0.25 && value_norm<0.5) Red   =0.0;
	if (value_norm>=0.5 && value_norm<0.75) Red   = (value_norm-0.5)*4;
	if (value_norm>=0.75)                   Red   =1.0;
    return Red;
}

double CGSPropertyRightResults::Get_Green_Value(double value_norm) 
{
	double Green=0.0;
	if (value_norm<0.25)                    Green =value_norm*4;
	if (value_norm>=0.25 && value_norm<0.5) Green =1.0;
	if (value_norm>=0.5 && value_norm<0.75) Green =1.0;
	if (value_norm>=0.75)                   Green = 1.0-((value_norm-0.75)*4);
    return Green;
}

double CGSPropertyRightResults::Get_Blue_Value(double value_norm) 
{
	double Blue=1.0;
	if (value_norm<0.25)                    Blue  =1.0;
	if (value_norm>=0.25 && value_norm<0.5) Blue  =1.0-((value_norm-0.25)*4);
	if (value_norm>=0.5 && value_norm<0.75) Blue  =0.0;
	if (value_norm>=0.75)                   Blue  =0.0;
    return Blue;
}