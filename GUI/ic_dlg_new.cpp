// DialogBoundaryConditionsNew.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "ic_dlg_new.h"


// CDialogInitialConditionsNew dialog

IMPLEMENT_DYNAMIC(CDialogInitialConditionsNew, CDialog)
CDialogInitialConditionsNew::CDialogInitialConditionsNew(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogInitialConditionsNew::IDD, pParent)
{
}

CDialogInitialConditionsNew::~CDialogInitialConditionsNew()
{
}

void CDialogInitialConditionsNew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PCS_TYPE, m_CB_PCSType);
    DDX_Control(pDX, IDC_COMBO_GEO_TYPE, m_CB_GEOType);
    DDX_Control(pDX, IDC_COMBO_DIS_TYPE, m_CB_DISType);
    DDX_Control(pDX, IDC_LIST_GEO, m_LB_GEO);
    DDX_Text(pDX, IDC_EDIT_VALUE, m_dValue);
    DDX_Control(pDX, IDC_LIST, m_LC);
}

BEGIN_MESSAGE_MAP(CDialogInitialConditionsNew, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PCS_TYPE, OnCbnSelchangeComboPCSType)
    ON_CBN_SELCHANGE(IDC_COMBO_GEO_TYPE, OnCbnSelchangeComboGEOType)
    ON_CBN_SELCHANGE(IDC_COMBO_DIS_TYPE, OnCbnSelchangeComboDISType)
    ON_LBN_SELCHANGE(IDC_LIST_GEO, OnLbnSelchangeListGEO)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_GROUP, OnBnClickedButtonCreateGroup)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_READ_IC, OnBnClickedButtonRead)
    ON_BN_CLICKED(IDC_BUTTON_READ_DATA, OnBnClickedButtonReadData)
    ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonWrite)
    ON_BN_CLICKED(IDC_BUTTON_WRITE_TEC, OnBnClickedButtonWriteTEC)
END_MESSAGE_MAP()


// CDialogInitialConditionsNew message handlers

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation based on CDialogBoundaryConditions
**************************************************************************/
/*
BOOL CDialogInitialConditionsNew::OnInitDialog() 
{
  CDialog::OnInitDialog();
  return TRUE;
}
*/
/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnCbnSelchangeComboPCSType()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnCbnSelchangeComboGEOType()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnLbnSelchangeListGEO()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnCbnSelchangeComboDISType()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonCreate()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonRemove()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonCreateGroup()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonRead()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonReadData()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonWrite()
{
}

/**************************************************************************
GUILib-Method:
Task:
Programing:
05/2005 OK Implementation
**************************************************************************/
void CDialogInitialConditionsNew::OnBnClickedButtonWriteTEC()
{
}

