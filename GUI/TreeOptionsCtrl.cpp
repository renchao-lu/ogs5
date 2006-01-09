//////////////// Includes ////////////////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "TreeOptionsCtrl.h"


//////////////// Macros / Locals /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT TREE_OPTIONS_COMBOBOX_ID = 100;
const UINT TREE_OPTIONS_EDITBOX_ID = 100;



//////////////// Implementation //////////////////////////////////////
IMPLEMENT_DYNAMIC(CTreeOptionsCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeOptionsCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeOptionsCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTreeOptionsCtrl::CTreeOptionsCtrl()
{
  m_pCombo = NULL;
  m_pEdit = NULL;
  m_nilID = IDB_TREE_CTRL_OPTIONS;
  m_hControlItem = NULL;
}

void CTreeOptionsCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
  UINT uFlags=0;
	HTREEITEM hItem = HitTest(point, &uFlags);

  //If the mouse was over the label, icon or to the left or right of the item ?
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
    else if (IsRadioButton(hItem))
    {
      BOOL bEnable;
      VERIFY(GetRadioButtonEnable(hItem, bEnable));
      if (bEnable)
      {
        //Check the radio button if not already checked
        BOOL bCheck;
        VERIFY(GetRadioButton(hItem, bCheck));
        if (!bCheck)
          VERIFY(SetRadioButton(hItem));
      }
    }

    //Pass on to the parent now that we that we have done our stuff
  	CTreeCtrl::OnLButtonDown(nFlags, point);
	}
  else
  {
    //Pass on to the parent since we didn't handle it
  	CTreeCtrl::OnLButtonDown(nFlags, point);
  }
}

void CTreeOptionsCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
 	if (nChar == VK_SPACE)
  {
		HTREEITEM hItem = GetSelectedItem();

    //If the space key is hit, and the item is a combo item, then toggle the check value
    if (IsCheckBox(hItem))
    {
      BOOL bEnable;
      VERIFY(GetCheckBoxEnable(hItem, bEnable));
      if (bEnable)
      {
        BOOL bCheck;
        VERIFY(GetCheckBox(hItem, bCheck));
        VERIFY(SetCheckBox(hItem, !bCheck));
      }
      else
      {
        //Pass on to the parent since we didn't handle it
        CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
      }
    }
    else if (IsRadioButton(hItem)) //If the item is a radio item, then check it and uncheck all other items
    {
      BOOL bEnable;
      VERIFY(GetRadioButtonEnable(hItem, bEnable));
      if (bEnable)
      {
        //Check the radio button if not already checked
        BOOL bCheck;
        VERIFY(GetRadioButton(hItem, bCheck));
        if (!bCheck)
          VERIFY(SetRadioButton(hItem));
      }
      else
      {
        //Pass on to the parent since we didn't handle it
        CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
      }
    }
    else
    {
      //Pass on to the parent since we didn't handle it
      CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
    }
  }
  else
  {
    //Pass on to the parent since we didn't handle it
  	CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
  }
}

HTREEITEM CTreeOptionsCtrl::InsertGroup(LPCTSTR lpszItem, int nImage, HTREEITEM hParent)
{
  ASSERT(nImage > 7); //You must specify an image index greater than three as the 
                      //first 8 images in the image list are reserved for the 
                      //checked and unchecked check box and radio buttons images

  HTREEITEM hItem = InsertItem(lpszItem, nImage, nImage, hParent);
  SetItemData(hItem, 0); //Item data must be 0
  return hItem;
}

HTREEITEM CTreeOptionsCtrl::InsertCheckBox(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck)
{
  ASSERT(IsGroup(hParent)); //The parent of a combo item must be a group item

  HTREEITEM hItem = InsertItem(lpszItem, 0, 0, hParent);
  SetItemData(hItem, 0);    //Item data must be 0
  BOOL bSuccess = SetCheckBox(hItem, bCheck);
  ASSERT(bSuccess);

  return hItem;
}

HTREEITEM CTreeOptionsCtrl::InsertRadioButton(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck)
{
  ASSERT(IsGroup(hParent)); //The parent of a radio item must be a group item

  HTREEITEM hItem = InsertItem(lpszItem, 2, 2, hParent);
  SetItemData(hItem, 0);    //Item data must be 0
  if (bCheck)
  {
    //if requested to, check the newly added radio button
    BOOL bSuccess = SetRadioButton(hItem);
    ASSERT(bSuccess);
  }

  return hItem;
}

BOOL CTreeOptionsCtrl::IsGroup(HTREEITEM hItem)
{
  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  return (nImage > 7);
}

BOOL CTreeOptionsCtrl::IsCheckBox(HTREEITEM hItem)
{
  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  return (nImage == 0 || nImage == 1 || nImage == 4 || nImage == 5);
}

BOOL CTreeOptionsCtrl::IsRadioButton(HTREEITEM hItem)
{
  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  return (nImage == 2 || nImage == 3 || nImage == 6 || nImage == 7);
}

BOOL CTreeOptionsCtrl::SetCheckBox(HTREEITEM hItem, BOOL bCheck)
{
  //Validate our parameters
  ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it

  BOOL bSuccess;
  if (bCheck)
    bSuccess = SetItemImage(hItem, 1, 1);
  else
    bSuccess = SetItemImage(hItem, 0, 0);   
  
  return bSuccess;
}

BOOL CTreeOptionsCtrl::GetCheckBox(HTREEITEM hItem, BOOL& bCheck)
{
  //Validate our parameters
  ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bCheck = (nImage == 1 || nImage == 5);

  return bSuccess;
}

BOOL CTreeOptionsCtrl::SetRadioButton(HTREEITEM hParent, int nIndex)
{
  //Validate our parameters
  ASSERT(IsGroup(hParent)); //Parent item must be a group item

  //Iterate through the child items and turn on the specified one and turn off all the other ones
  HTREEITEM hChild = GetNextItem(hParent, TVGN_CHILD);

  //Turn of redraw to Q all the changes we're going to make here
  SetRedraw(FALSE);
  
  int i=0;
  BOOL bCheckedSomeItem = FALSE;
  while (hChild)
  {
    ASSERT(IsRadioButton(hChild)); //Child item must be a radio item

    if (i == nIndex)
    {
      //Turn this item on
      VERIFY(SetItemImage(hChild, 3, 3));
      bCheckedSomeItem = TRUE;
    }
    else
    {
      BOOL bEnable;
      VERIFY(GetRadioButtonEnable(hChild, bEnable));

      //Turn this item off
      if (bEnable)
        VERIFY(SetItemImage(hChild, 2, 2));
      else
        VERIFY(SetItemImage(hChild, 4, 4));
    }

    //Move on to the next item
    hChild = GetNextItem(hChild, TVGN_NEXT);
    ++i;
  }
  ASSERT(bCheckedSomeItem); //You specified an index which does not exist

  //Reset the redraw flag
  SetRedraw(TRUE);

  return TRUE;
}

BOOL CTreeOptionsCtrl::SetRadioButton(HTREEITEM hItem)
{
  //Validate our parameters
  ASSERT(IsRadioButton(hItem)); //Must be a radio item to check it

  //Iterate through the sibling items and turn them all off except this one
  HTREEITEM hParent = GetNextItem(hItem, TVGN_PARENT);
  ASSERT(IsGroup(hParent)); //Parent item must be a group item

  //Iterate through the child items and turn on the specified one and turn off all the other ones
  HTREEITEM hChild = GetNextItem(hParent, TVGN_CHILD);

  //Turn of redraw to Q all the changes we're going to make here
  SetRedraw(FALSE);

  while (hChild)
  {
    ASSERT(IsRadioButton(hChild)); //Child item must be a radio item

    if (hChild == hItem)
    {
      //Turn this item on
      BOOL bSuccess = SetItemImage(hChild, 3, 3);
      ASSERT(bSuccess);
    }
    else
    {
      BOOL bEnable;
      VERIFY(GetRadioButtonEnable(hChild, bEnable));

      //Turn this item off
      if (bEnable)
        VERIFY(SetItemImage(hChild, 2, 2));
      else
        VERIFY(SetItemImage(hChild, 6, 6));
    }

    //Move on to the next item
    hChild = GetNextItem(hChild, TVGN_NEXT);
  }

  //Reset the redraw flag
  SetRedraw(TRUE);

  return TRUE;
}

BOOL CTreeOptionsCtrl::GetRadioButton(HTREEITEM hParent, int& nIndex, HTREEITEM& hCheckItem)
{
  ASSERT(IsGroup(hParent)); //Parent item must be a group item

  //Iterate through the child items and turn on the specified one and turn off all the other ones
  HTREEITEM hChild = GetNextItem(hParent, TVGN_CHILD);
  ASSERT(hChild); //Must have some child items

  //Find the checked item  
  nIndex = 0;
  BOOL bFound = FALSE;
  while (!bFound)
  {
    BOOL bSuccess = GetRadioButton(hChild, bFound);
    ASSERT(bSuccess);

    //Move on to the next sibling if not found
    if (!bFound)
    {
      hChild = GetNextItem(hChild, TVGN_NEXT);
      ASSERT(hChild);
      ++nIndex;
    }
    else
      hCheckItem = hChild;
  }

  return TRUE;
}

BOOL CTreeOptionsCtrl::GetRadioButton(HTREEITEM hItem, BOOL& bCheck)
{
  ASSERT(IsRadioButton(hItem)); //Must be a radio item to check it

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bCheck = (nImage == 3 || nImage == 7);

  return bSuccess;
}

BOOL CTreeOptionsCtrl::SetGroupEnable(HTREEITEM hItem, BOOL bEnable)
{
  //Allows you to quickly enable / disable all the items in a group

  ASSERT(IsGroup(hItem)); //Must be group item

  //Iterate through the child items and enable / disable all the items
  HTREEITEM hChild = GetNextItem(hItem, TVGN_CHILD);

  //Turn of redraw to Q all the changes we're going to make here
  SetRedraw(FALSE);

  while (hChild)
  {
    if (IsRadioButton(hChild))
    {
      int nImage;
      int nSelectedImage;
      VERIFY(GetItemImage(hChild, nImage, nSelectedImage));
      BOOL bCheck = (nImage == 3 || nImage == 7);
      if (bCheck)
      {
        if (bEnable)
          VERIFY(SetItemImage(hChild, 3, 3));
        else
          VERIFY(SetItemImage(hChild, 7, 7));
      }
      else
      {
        if (bEnable)
          VERIFY(SetItemImage(hChild, 2, 2));
        else
          VERIFY(SetItemImage(hChild, 6, 6));
      }
    }
    else if (IsCheckBox(hChild))
      VERIFY(SetCheckBoxEnable(hChild, bEnable));
    else
      ASSERT(FALSE);

    //Move onto the next child
    hChild = GetNextItem(hChild, TVGN_NEXT);
  }

  //Reset the redraw flag
  SetRedraw(TRUE);

  return TRUE;
}

BOOL CTreeOptionsCtrl::SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable)
{
  ASSERT(IsCheckBox(hItem)); //Must be a check box
  BOOL bSuccess = FALSE;

  if (bEnable)
  {
    BOOL bCheck;
    VERIFY(GetCheckBox(hItem, bCheck));
    if (bCheck)
      bSuccess = SetItemImage(hItem, 1, 1);
    else
      bSuccess = SetItemImage(hItem, 0, 0);
  }
  else
  {
    BOOL bCheck;
    VERIFY(GetCheckBox(hItem, bCheck));
    if (bCheck)
      bSuccess = SetItemImage(hItem, 5, 5);
    else
      bSuccess = SetItemImage(hItem, 4, 4);
  }

  return bSuccess;
}

BOOL CTreeOptionsCtrl::SetRadioButtonEnable(HTREEITEM hItem, BOOL bEnable)
{
  ASSERT(IsRadioButton(hItem)); //Must be a radio button
  BOOL bSuccess = FALSE;

  if (bEnable)
  {
    BOOL bCheck;
    VERIFY(GetRadioButton(hItem, bCheck));
    if (bCheck)
      bSuccess = SetItemImage(hItem, 3, 3);
    else
      bSuccess = SetItemImage(hItem, 2, 2);
  }
  else
  {
    BOOL bCheck;
    VERIFY(GetRadioButton(hItem, bCheck));
    if (bCheck)
      bSuccess = SetItemImage(hItem, 7, 7);
    else
      bSuccess = SetItemImage(hItem, 6, 6);
  }

  return bSuccess;
}

BOOL CTreeOptionsCtrl::GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable)
{
  ASSERT(IsCheckBox(hItem)); //Must be a check box

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bEnable = (nImage == 0 || nImage == 1);

  return bSuccess;  
}

BOOL CTreeOptionsCtrl::GetRadioButtonEnable(HTREEITEM hItem, BOOL& bEnable)
{
  ASSERT(IsRadioButton(hItem)); //Must be a radio button

  int nImage;
  int nSelectedImage;
  BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);
  ASSERT(bSuccess);

  bEnable = (nImage == 2 || nImage == 3);

  return bSuccess;  
}

void CTreeOptionsCtrl::PreSubclassWindow() 
{
  //Let the parent class do its thing	
	CTreeCtrl::PreSubclassWindow();

  //Loadup the image list
  VERIFY(m_ilTree.Create(m_nilID, 16, 1, RGB(255, 0, 255)));

  //Hook it up to the tree control
  SetImageList(&m_ilTree, TVSIL_NORMAL);
}

BOOL CTreeOptionsCtrl::AddComboBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClass)
{
  //Musn't be setup as one already
  ASSERT(!IsRadioButton(hItem)); 
  ASSERT(!IsCheckBox(hItem)); 
  
  //Pointer to the runtime class is stored in the Item data  
  return SetItemData(hItem, (DWORD_PTR) pRuntimeClass);
}

CString CTreeOptionsCtrl::GetComboText(HTREEITEM hItem)
{
  CString sText = GetItemText(hItem);
  CString sOriginalText = sText;
  int nColon = sText.Find(_T(": "));
  CString sComboText;
  if (nColon != -1)
  {
    sComboText = sText.Right(sOriginalText.GetLength() - nColon - 2);
    sOriginalText = sText.Left(nColon + 2);
  }
  else
    sOriginalText += _T(": ");
  if (sText != sOriginalText)
    SetItemText(hItem, sOriginalText);

  return sComboText;
}

void CTreeOptionsCtrl::SetComboText(HTREEITEM hItem, const CString& sComboText)
{
  CString sText = GetItemText(hItem);
  int nColon = sText.Find(_T(": "));
  if (nColon == -1)
  {
    sText += _T(": ");
    SetItemText(hItem, sText);
  }
  sText += sComboText;
  SetItemText(hItem, sText);
}

void CTreeOptionsCtrl::DestroyOldChildControl()
{
  if (m_pCombo)
  {
    m_pCombo->DestroyWindow();
    delete m_pCombo;
    m_pCombo = NULL;
  }
  if (m_pEdit)
  {
    m_pEdit->DestroyWindow();
    delete m_pEdit;
    m_pEdit = NULL;
  }
  m_hControlItem = NULL;
}

void CTreeOptionsCtrl::CreateNewChildControl(HTREEITEM hItem)
{
  ASSERT(hItem);
  m_hControlItem = hItem;

  CFont* pFont = GetFont();
  CRuntimeClass* pRuntimeClass = (CRuntimeClass*) GetItemData(hItem);
  ASSERT(pRuntimeClass);
  if (pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(CTreeOptionsCombo)))
  {
    //Get the current text in the combo item
    CString sComboText = GetComboText(hItem);

    //work out the rect for the combo box
    CRect rText;
    GetItemRect(hItem, &rText, TRUE);
    CRect rLine;
    GetItemRect(hItem, &rLine, FALSE);
    CRect r;
    r.top = rText.top;
    r.left = rText.right + 2;

    //Create the new combo box
    m_pCombo = (CTreeOptionsCombo*) pRuntimeClass->CreateObject();
    ASSERT(m_pCombo);
    ASSERT(m_pCombo->IsKindOf(RUNTIME_CLASS(CTreeOptionsCombo)));  //Your class must be derived from CTreeOptionsCombo
    m_pCombo->Init(this);
    r.right = r.left + m_pCombo->GetWidth(); //Ask the combo box for the width to use
    r.bottom = r.top + m_pCombo->GetDropDownHeight(); //Ask the combo box for the height to use
    m_pCombo->Create(m_pCombo->GetWindowStyle(), r, this, TREE_OPTIONS_COMBOBOX_ID);
    ASSERT(m_pCombo->GetCount()); //You forget to add string items to the combo box in your OnCreate message handler!

    //set the font the combo box should use based on the font in the tree control, 
    m_pCombo->SetFont(pFont);

    //Also select the right text into the combo box
    m_pCombo->SelectString(-1, sComboText);
  }
  else if (pRuntimeClass->IsDerivedFrom(RUNTIME_CLASS(CTreeOptionsEdit)))
  {
    //Get the current text in the edit box item
    CString sEditText = GetEditText(hItem);

    //work out the rect for the combo box
    CRect rText;
    GetItemRect(hItem, &rText, TRUE);
    CRect rLine;
    GetItemRect(hItem, &rLine, FALSE);
    CRect r;
    r.top = rText.top;
    r.left = rText.right + 2;

    //Create the new edit box
    m_pEdit = (CTreeOptionsEdit*) pRuntimeClass->CreateObject();
    ASSERT(m_pEdit);
    ASSERT(m_pEdit->IsKindOf(RUNTIME_CLASS(CTreeOptionsEdit)));  //Your class must be derived from CTreeOptionsEdit
    m_pEdit->Init(this);
    r.right = r.left + m_pEdit->GetWidth(); //Ask the edit box for the width to use
    r.bottom = rText.bottom + 5;
    m_pEdit->CreateEx(WS_EX_CLIENTEDGE, _T("Edit"), sEditText, m_pEdit->GetWindowStyle(), r, this, TREE_OPTIONS_EDITBOX_ID);

    //set the font the edit box should use based on the font in the tree control
    m_pEdit->SetFont(pFont);
  }
  else
    ASSERT(FALSE); //Your class must be derived from CTreeOptionsCombo or CTreeOptionsEdit
}

void CTreeOptionsCtrl::UpdateTreeControlTextFromChildControl()
{
  CString sText;
  if (m_pCombo)
  {
    m_pCombo->GetWindowText(sText);
    SetComboText(m_hControlItem, sText);
  }
  else
  {
    ASSERT(m_pEdit);
    m_pEdit->GetWindowText(sText);
    SetEditText(m_hControlItem, sText);
  }
}

BOOL CTreeOptionsCtrl::AddEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClass)
{
  //Just call the combo box version as currently there is no difference
  return AddComboBox(hItem, pRuntimeClass);
}

CString CTreeOptionsCtrl::GetEditText(HTREEITEM hItem)
{
  //Just call the combo box version as currently there is no difference
  return GetComboText(hItem);
}

void CTreeOptionsCtrl::SetEditText(HTREEITEM hItem, const CString& sEditText)
{
  //Just call the combo box version as currently there is no difference
  SetComboText(hItem, sEditText);
}

void CTreeOptionsCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

  //Destroy the old combo or edit box if need be
  if (m_hControlItem)
  {
    UpdateTreeControlTextFromChildControl();
    DestroyOldChildControl();
    m_hControlItem = NULL;
  }

  //Create the new combo if need be
  if (GetItemData(pNMTreeView->itemNew.hItem))
    CreateNewChildControl(pNMTreeView->itemNew.hItem);

	*pResult = 0;
}

void CTreeOptionsCtrl::OnDestroy() 
{
  //Clean up any memory we used
  DestroyOldChildControl();

  //Let the parent class do its thing
	CTreeCtrl::OnDestroy();
}

void CTreeOptionsCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  //Clean up any controls currently open we used
  if (m_hControlItem)
  {
    UpdateTreeControlTextFromChildControl();
    DestroyOldChildControl();
  }
	
  //Let the parent class do its thing
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CTreeOptionsCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  //Clean up any controls currently open we used
  if (m_hControlItem)
  {
    UpdateTreeControlTextFromChildControl();
    DestroyOldChildControl();
  }
	
  //Let the parent class do its thing
	CTreeCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTreeOptionsCtrl::OnClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
  //If the mouse was over the label or icon and the item is a combo box
  //or edit box and editing is currently not active then create the 
  //new control
  UINT uFlags=0;
  CPoint point = GetCurrentMessage()->pt;
  ScreenToClient(&point);
	HTREEITEM hItem = HitTest(point, &uFlags);
	if ((uFlags & TVHT_ONITEM) && GetItemData(hItem) && m_hControlItem == NULL)
    CreateNewChildControl(hItem);
	
	*pResult = 0;
}




IMPLEMENT_DYNCREATE(CTreeOptionsCombo, CComboBox)

CTreeOptionsCombo::CTreeOptionsCombo()
{
  m_pTreeCtrl = NULL;
}

CTreeOptionsCombo::~CTreeOptionsCombo()
{
}

BEGIN_MESSAGE_MAP(CTreeOptionsCombo, CComboBox)
	//{{AFX_MSG_MAP(CTreeOptionsCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

DWORD CTreeOptionsCombo::GetWindowStyle()
{
  return WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;
}

int CTreeOptionsCombo::GetWidth()
{
  return 100;
}

int CTreeOptionsCombo::GetDropDownHeight()
{
  return 100;
}




IMPLEMENT_DYNCREATE(CTreeOptionsEdit, CEdit)

CTreeOptionsEdit::CTreeOptionsEdit()
{
  m_pTreeCtrl = NULL;
}

CTreeOptionsEdit::~CTreeOptionsEdit()
{
}

BEGIN_MESSAGE_MAP(CTreeOptionsEdit, CEdit)
	//{{AFX_MSG_MAP(CTreeOptionsEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

DWORD CTreeOptionsEdit::GetWindowStyle()
{
  return WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL;
}

int CTreeOptionsEdit::GetWidth()
{
  return 100;
}




void DDX_TreeCheck(CDataExchange* pDX, int nIDC, HTREEITEM hItem, BOOL& bCheck)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
  CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
  ASSERT(pCtrlTreeOptions);
  ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

  if (pDX->m_bSaveAndValidate)
  {
    BOOL bSuccess = pCtrlTreeOptions->GetCheckBox(hItem, bCheck);
    ASSERT(bSuccess);
  }
  else
  {
    BOOL bSuccess = pCtrlTreeOptions->SetCheckBox(hItem, bCheck);
    ASSERT(bSuccess);
  }
}

void DDX_TreeRadio(CDataExchange* pDX, int nIDC, HTREEITEM hParent, int& nIndex)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
  CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
  ASSERT(pCtrlTreeOptions);
  ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

  if (pDX->m_bSaveAndValidate)
  {
    HTREEITEM hCheckItem;
    BOOL bSuccess = pCtrlTreeOptions->GetRadioButton(hParent, nIndex, hCheckItem);
    ASSERT(bSuccess);
  }
  else
  {
    BOOL bSuccess = pCtrlTreeOptions->SetRadioButton(hParent, nIndex);
    ASSERT(bSuccess);
  }
}

void DDX_TreeEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
  CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
  ASSERT(pCtrlTreeOptions);
  ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

  if (pDX->m_bSaveAndValidate)
    sText = pCtrlTreeOptions->GetEditText(hItem);
  else
    pCtrlTreeOptions->SetEditText(hItem, sText);
}

void DDX_TreeCombo(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
  CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
  ASSERT(pCtrlTreeOptions);
  ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

  if (pDX->m_bSaveAndValidate)
    sText = pCtrlTreeOptions->GetComboText(hItem);
  else
    pCtrlTreeOptions->SetComboText(hItem, sText);
}

