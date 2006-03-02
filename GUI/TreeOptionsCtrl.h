/*
Module : TreeOptionsCtrl.H
Purpose: Defines the interface for an MFC class to implement a tree options control 
         similiar to the advanced tab as seen on the "Internet options" dialog in 
         Internet Explorer 4 and later
Created: PJN / 31-03-1999

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////
#ifndef __TREEOPTIONSCTRL_H__
#define __TREEOPTIONSCTRL_H__


/////////////////////////////// Classes ///////////////////////////////////////

//forward declaration
class CTreeOptionsCtrl;



//Class which represents a combo box used by the tree options class
class CTreeOptionsCombo : public CComboBox
{
public:
	CTreeOptionsCombo();
	virtual ~CTreeOptionsCombo();

protected:
  void Init(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
  virtual DWORD GetWindowStyle();
  virtual int GetWidth();
  virtual int GetDropDownHeight();

	//{{AFX_VIRTUAL(CTreeOptionsCombo)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsCombo)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
  DECLARE_DYNCREATE(CTreeOptionsCombo)

  CTreeOptionsCtrl* m_pTreeCtrl;
  friend class CTreeOptionsCtrl;
};



//Class which represents an edit box used by the tree options class
class CTreeOptionsEdit : public CEdit
{
public:
	CTreeOptionsEdit();
	virtual ~CTreeOptionsEdit();

protected:
  void Init(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
  virtual DWORD GetWindowStyle();
  virtual int GetWidth();

	//{{AFX_VIRTUAL(CTreeOptionsEdit)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsEdit)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
  DECLARE_DYNCREATE(CTreeOptionsEdit)

  CTreeOptionsCtrl* m_pTreeCtrl;
  friend class CTreeOptionsCtrl;
};



//The actual tree options control class
class CTreeOptionsCtrl : public CTreeCtrl
{
public:
//Constructors / Destructors
  CTreeOptionsCtrl();

//Misc
  void SetImageListToUse(UINT nResourceID) { m_nilID = nResourceID; };


//Inserting items into the control
  HTREEITEM InsertGroup(LPCTSTR lpszItem, int nImage, HTREEITEM hParent = TVI_ROOT);
  HTREEITEM InsertCheckBox(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck = TRUE);
  HTREEITEM InsertRadioButton(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck = TRUE);

//Validation methods
  BOOL IsGroup(HTREEITEM hItem);
  BOOL IsCheckBox(HTREEITEM hItem);
  BOOL IsRadioButton(HTREEITEM hItem);

//Setting / Getting combo states
  BOOL SetCheckBox(HTREEITEM hItem, BOOL bCheck);
  BOOL GetCheckBox(HTREEITEM hItem, BOOL& bCheck);

//Setting / Getting radio states
  BOOL SetRadioButton(HTREEITEM hParent, int nIndex);
  BOOL SetRadioButton(HTREEITEM hItem);
  BOOL GetRadioButton(HTREEITEM hParent, int& nIndex, HTREEITEM& hCheckItem);
  BOOL GetRadioButton(HTREEITEM hItem, BOOL& bCheck);

//Enable / Disbale items
  BOOL SetGroupEnable(HTREEITEM hItem, BOOL bEnable);
  BOOL SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable);
  BOOL SetRadioButtonEnable(HTREEITEM hItem, BOOL bEnable);  
  BOOL GetRadioButtonEnable(HTREEITEM hItem, BOOL& bEnable);
  BOOL GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable);

//Adding a combo box to an item
  BOOL    AddComboBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClass);
  CString GetComboText(HTREEITEM hItem);
  void    SetComboText(HTREEITEM hItem, const CString& sComboText);

//Adding an edit box to an item
  BOOL    AddEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClass);
  CString GetEditText(HTREEITEM hItem);
  void    SetEditText(HTREEITEM hItem, const CString& sEditText);

protected:
  CImageList m_ilTree;
  UINT m_nilID;
  CTreeOptionsCombo* m_pCombo;
  CTreeOptionsEdit* m_pEdit;
  HTREEITEM m_hControlItem;
  void DestroyOldChildControl();
  void CreateNewChildControl(HTREEITEM hItem);
  void UpdateTreeControlTextFromChildControl();

	//{{AFX_VIRTUAL(CTreeOptionsCtrl)
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

  DECLARE_DYNAMIC(CTreeOptionsCtrl)

	DECLARE_MESSAGE_MAP()
};

//Dialog Data exchange support
void DDX_TreeCheck(CDataExchange* pDX, int nIDC, HTREEITEM hItem, BOOL& bCheck);
void DDX_TreeRadio(CDataExchange* pDX, int nIDC, HTREEITEM hParent, int& nIndex);
void DDX_TreeEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_TreeCombo(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);



#endif //__TREEOPTIONSCTRL_H__



