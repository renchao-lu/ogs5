// PropPgFormDoc.cpp : implementation of the CPropPgFormDoc class
//

#include "stdafx.h"
#include "GeoSys.h"

#include "PropPgFormDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormDoc

IMPLEMENT_DYNCREATE(CPropPgFormDoc, CDocument)

BEGIN_MESSAGE_MAP(CPropPgFormDoc, CDocument)
	//{{AFX_MSG_MAP(CPropPgFormDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormDoc construction/destruction

CPropPgFormDoc::CPropPgFormDoc()
{
}

CPropPgFormDoc::~CPropPgFormDoc()
{
}

BOOL CPropPgFormDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPropPgFormDoc serialization

void CPropPgFormDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormDoc diagnostics

#ifdef _DEBUG
void CPropPgFormDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPropPgFormDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropPgFormDoc commands
