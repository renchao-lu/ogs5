// gs_newproject.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_newproject.h"
#include <Shlobj.h>
#include ".\gs_newproject.h"

// CGS_newproject dialog

IMPLEMENT_DYNAMIC(CGS_newproject, CDialog)
CGS_newproject::CGS_newproject(CWnd* pParent /*=NULL*/)
	: CDialog(CGS_newproject::IDD, pParent)
{
}

CGS_newproject::~CGS_newproject()
{
}

void CGS_newproject::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDemoDlg)
	DDX_Text(pDX, IDC_TXT_FOLDERPATH, m_strFolderPath);
	DDX_Text(pDX, IDC_PROJECT_NAME, m_strProjectName);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CGS_newproject, CDialog)
	ON_BN_CLICKED(IDC_CMD_BROWSE, OnPRJBrowse)
    ON_EN_CHANGE(IDC_PROJECT_NAME, OnEnChangeProjectName)
END_MESSAGE_MAP()


// CGS_newproject message handlers

void CGS_newproject::OnPRJBrowse( void ) 
{
	BROWSEINFO bi;
    TCHAR szDir[MAX_PATH];
    LPITEMIDLIST pidl;
    LPMALLOC pMalloc;

	UpdateData( TRUE);

    if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
    {

        ZeroMemory(&bi,sizeof(bi));
        bi.pszDisplayName = new TCHAR[ MAX_PATH ];
        bi.lpszTitle = _T( "Select the project folder:\n \n The project file (*.gsp) will be installed in this directory" );
        bi.ulFlags = BIF_STATUSTEXT | BIF_EDITBOX |BIF_NEWDIALOGSTYLE;
 
        pidl = SHBrowseForFolder(&bi);
        if (pidl) SHGetPathFromIDList(pidl,szDir);
		m_strFolderPathBasic = szDir ;
        CString slash = m_strFolderPathBasic.Right(1);
        if (slash == "\\") m_strFolderPathBasic = m_strFolderPathBasic.Left(m_strFolderPathBasic.GetAllocLength() -1);
		m_strFolderPath = m_strFolderPathBasic + '\\' + m_strProjectName;
		

		UpdateData( FALSE );

        pMalloc->Free(pidl); 
        pMalloc->Release();
	}
 

}

void CGS_newproject::OnEnChangeProjectName()
{
    UpdateData( TRUE);
    if (m_strFolderPathBasic.GetLength()== 0) m_strFolderPathBasic = "C:";
    m_strFolderPath = m_strFolderPathBasic + '\\' + m_strProjectName;
	UpdateData( FALSE);	
}



