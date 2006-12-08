// IsoControl.cpp : implementation file
// Author: HS Control for Isosurface function

#include "stdafx.h"
#include "afxpriv.h"
#include "Windowsx.h"
#include "GeoSys.h"
#include "IsoControl.h"
#include "DlgIsoListValue.h"
#include "DlgIsoFrameWidth.h"
#include "GeoSysDoc.h"
#include "MainFrm.h"
#include "GSPropertyRightResults.h"
#include "GSForm3DLeft.h"
#include "GeoSysTreeView.h"
#include "COGLPickingView.h"

// CIsoControl dialog

IMPLEMENT_DYNAMIC(CIsoControl, CDialog)
CIsoControl::CIsoControl(CWnd* pParent /*=NULL*/)
	: CDialog(CIsoControl::IDD, pParent)
{
    m_pParent = pParent;
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

CIsoControl::~CIsoControl()
{
}
BOOL CIsoControl::OnInitDialog()
{
    CDialog::OnInitDialog();

     CRect rect;
     m_Iso_List.GetClientRect(&rect);
      int nColInterval = rect.Width()/10;

     m_Iso_List.InsertColumn(0, _T("#"), LVCFMT_LEFT, nColInterval*2);
     m_Iso_List.InsertColumn(1, _T("Value"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List.InsertColumn(2, _T("Iso_Col"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List.InsertColumn(3, _T("Frame_Col"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List.InsertColumn(4, _T("Width"), LVCFMT_LEFT, nColInterval*3);
     m_Iso_List.InsertColumn(5, _T("Line"), LVCFMT_LEFT, rect.Width()-8*nColInterval);
     m_Iso_List.ModifyStyle(0, LVS_REPORT);

     ListView_SetExtendedListViewStyle(::GetDlgItem(m_hWnd,IDC_LIST6),LVS_EX_FULLROWSELECT | 
        LVS_EX_GRIDLINES); 

     ::ShowWindow(::GetDlgItem(m_hWnd,IDC_EDIT1),SW_HIDE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
void CIsoControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CHECK_If_Show_Iso, Control_Show_Iso);
	DDX_Control(pDX, IDC_CHECK3, Control_SwitchOff_ContourPlot);
	//DDX_Radio(pDX, IDC_RADIO_IsoLines, Iso_Form);
	//DDX_Radio(pDX, IDC_RADIO_SURFACE_TYPE1, Iso_Type);
	DDX_Control(pDX, IDC_LIST6, m_Iso_List);
	DDX_Text(pDX,IDC_EDIT_Num, Iso_Num);
}


BEGIN_MESSAGE_MAP(CIsoControl, CDialog)
	ON_BN_CLICKED(IDC_CHECK_If_Show_Iso, OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_Iso_Refresh, OnBnClickedIsoRefresh)
	ON_BN_CLICKED(IDC_Iso_AddListItem, OnBnClickedIsoAddlistitem)
	ON_BN_CLICKED(IDC_Iso_DelListItem, OnBnClickedIsoDellistitem)
	ON_BN_CLICKED(IDOK, OnBnClickedIsoApply)
	ON_NOTIFY( NM_CUSTOMDRAW, IDC_LIST6, OnCustomdrawMyIsoList )
	ON_NOTIFY( NM_CLICK, IDC_LIST6, OnClickMyIsoList )
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillFocusIsoValueEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST6, OnNMDblclkList6)
	ON_WM_KEYDOWN()
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickIsoListCtrl)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CIsoControl message handlers
void CIsoControl::OnBnClickedCheck2()
{
    ((CGSPropertyRightResults*)m_pParent)->OnBnClickedGetPcsMinmaxButton3();
	UpdateData(true);
	if (Control_Show_Iso.GetCheck() == BST_CHECKED)
	{
		Iso_If_Show_Iso = true;
	}
	else
	{
		Iso_If_Show_Iso = false;
	}
	OnBnClickedIsoRefresh();
	UpdateData(false);


}

void CIsoControl::OnBnClickedCheck3()
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
void CIsoControl::InsertIsoListItem(double max, double min, int Num)
{
    m_Iso_List.DeleteAllItems();

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
		m_Iso_List.InsertItem(&lvi);

		// Set subitem 1
		strItem.Format(_T("%f"), min + (max-min)/Num*((double)(i+1)));
		lvi.iSubItem =1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);

		// Set subitem 2
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);

		// Set subitem 3
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);

		// Set subitem 4
		strItem.Format(_T("%1.1f"), IsoFrameWidth[i]);
		lvi.iSubItem =4;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);


	}

}
void CIsoControl::OnBnClickedIsoRefresh()
{
	UpdateData(true);
	CString str;

	((CGSPropertyRightResults*)m_pParent)->GetPcsMinmax();
	Iso_Min_Value = ((CGSPropertyRightResults*)m_pParent)->m_pcs_min_r;
	Iso_Max_Value = ((CGSPropertyRightResults*)m_pParent)->m_pcs_max_r;
	Iso_Step_Value = (Iso_Max_Value-Iso_Min_Value)/Iso_Num;

	str.Format("%f",Iso_Min_Value);
	SetDlgItemText(IDC_STATIC_MinValue,str);
	str.Format("%f",Iso_Max_Value);
	SetDlgItemText(IDC_STATIC_Max_Value,str);
	InsertIsoListItem(Iso_Max_Value,Iso_Min_Value,Iso_Num);
}

void CIsoControl::OnBnClickedIsoAddlistitem()
{
	int count;
	LVITEM lvi;
	CString strItem;
	count = m_Iso_List.GetItemCount();
		
	lvi.mask =  LVIF_TEXT;
	strItem.Format(_T(" %i"), count);

	lvi.iItem = count;
	lvi.iSubItem = 0;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List.InsertItem(&lvi);

	// Set subitem 1
	strItem.Format(_T("%f"), 0);
	lvi.iSubItem =1;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List.SetItem(&lvi);

	// Set subitem 2
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =2;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List.SetItem(&lvi);

	// Set subitem 3
	strItem.Format(_T("%s"), "");
	lvi.iSubItem =3;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List.SetItem(&lvi);

	// Set subitem 4
	strItem.Format(_T("%1.1f"), 1.0);
	lvi.iSubItem =4;
	lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
	m_Iso_List.SetItem(&lvi);
	
}

void CIsoControl::OnBnClickedIsoDellistitem()
{
	int mark;
	mark = m_Iso_List.GetSelectionMark();
	m_Iso_List.DeleteItem(mark);
}

void CIsoControl::OnBnClickedIsoApply()
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
	count =  m_Iso_List.GetItemCount();
	if (count <= 1000)
	{
		m_frame->Iso_Count = count;
		for(int i=0;i<count;i++)
		{
			CString str;
			str = m_Iso_List.GetItemText( i , 1 );
			m_frame->IsoValueArray[i] = atof(str.GetBuffer());
			m_frame->IsoColorArray[i] = IsoColor[i];
			m_frame->IsoFrameColorArray[i] = IsoFrameColor[i];
			str = m_Iso_List.GetItemText( i, 4 );
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
void CIsoControl::OnCustomdrawMyIsoList( NMHDR* pNMHDR, LRESULT* pResult )
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
void CIsoControl::SetCell(HWND hWnd1, CString value, int nRow, int nCol)
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
CString CIsoControl::GetItemText(HWND hWnd, int nItem, int nSubItem) const
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
void CIsoControl::OnClickMyIsoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    Invalidate();
    HWND hWnd1 =  ::GetDlgItem (m_hWnd,IDC_LIST6);
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

void CIsoControl::OnKillFocusIsoValueEdit()
{
    CWnd* pwndCtrl = GetFocus();
    // get the control ID which is presently having the focus
	int ctrl_ID = pwndCtrl->GetDlgCtrlID();
	CString str;
	//get the text from the EditBox
	GetDlgItemText(IDC_EDIT1,str);
	//set the value in the listContorl with the specified Item & SubItem values
	SetCell(::GetDlgItem (m_hWnd,IDC_LIST6),str,nItem,nSubItem);
	::SendDlgItemMessage(m_hWnd,IDC_EDIT1,WM_KILLFOCUS,0,0);
	::ShowWindow(::GetDlgItem(m_hWnd,IDC_EDIT1),SW_HIDE);
}

void CIsoControl::OnNMDblclkList6(NMHDR *pNMHDR, LRESULT *pResult)
{
    Invalidate();
    HWND hWnd1 =  ::GetDlgItem (m_hWnd,IDC_LIST6);
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
			m_Iso_List.RedrawItems(temp->iItem,temp->iItem);
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
			m_Iso_List.RedrawItems(temp->iItem,temp->iItem);
		}
	}
	*pResult = 0;
}

void CIsoControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	nFlags = nFlags;
	nRepCnt = nRepCnt;
	if(nChar==VK_RETURN){
		OnKillFocusIsoValueEdit();
	}

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CIsoControl::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
	OnKeyDown((UINT)pMsg->wParam,(UINT)pMsg->lParam,(UINT)pMsg->lParam);
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CIsoControl::OnItemclickIsoListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	NMHEADER *pHdr = (NMHEADER*)pNMHDR;

	// Get a pointer to the header control and figure out what happened
	CHeaderCtrl *pHeader = m_Iso_List.GetHeaderCtrl();
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
				CColorDialog dlg;
				if ( dlg.DoModal() == IDOK )
				{
					int count;
					count =  m_Iso_List.GetItemCount();
					for (int i=0;i < count ;i++)
					{
						IsoColor[i]=dlg.GetColor();
					}
					m_Iso_List.RedrawItems(0,count);
				}
			break;
			}
		case 3:
			{
				CColorDialog dlg;
				if ( dlg.DoModal() == IDOK )
				{
					int count;
					count =  m_Iso_List.GetItemCount();
					for (int i=0;i < count ;i++)
					{
						IsoFrameColor[i]=dlg.GetColor();
					}
					m_Iso_List.RedrawItems(0,count);
				}
			break;
			}
		case 4:
			{
				CDlgIsoFrameWidth dlg;
				if ( dlg.DoModal() == IDOK )
				{
					int count;
					count =  m_Iso_List.GetItemCount();
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
void CIsoControl::InsertIsoListItem(double max, double min, double step)
{
    m_Iso_List.DeleteAllItems();
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
		m_Iso_List.InsertItem(&lvi);

		// Set subitem 1
		strItem.Format(_T("%f"), min+i*step);
		lvi.iSubItem =1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);

		// Set subitem 2
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);

		// Set subitem 3
		strItem.Format(_T("%s"), "");
		lvi.iSubItem =3;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);		
		
		// Set subitem 4
		strItem.Format(_T("%1.1f"), IsoFrameWidth[i]);
		lvi.iSubItem =4;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_Iso_List.SetItem(&lvi);
	}
}

void CIsoControl::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	if(m_pParent)
	{		
		((CGSPropertyRightResults*)m_pParent)->m_pmodeless_r = NULL;		
	}
	delete this;
}

void CIsoControl::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    DestroyWindow();
}
