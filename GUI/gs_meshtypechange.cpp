// gs_meshtypechange.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_meshtypechange.h"


// CMeshTypeChange dialog

IMPLEMENT_DYNAMIC(CMeshTypeChange, CDialog)
CMeshTypeChange::CMeshTypeChange(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshTypeChange::IDD, pParent)
{
   	m_numberofprismlayers = 0;
	m_thicknessofprismlayer = 0;

}

CMeshTypeChange::~CMeshTypeChange()
{
}

void CMeshTypeChange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_LAYERNUMBER, m_numberofprismlayers);
    DDX_Text(pDX, IDC_EDIT_LAYERTHICKNESS, m_thicknessofprismlayer);

}


BEGIN_MESSAGE_MAP(CMeshTypeChange, CDialog)
END_MESSAGE_MAP()


// CMeshTypeChange message handlers
