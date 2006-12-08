// CPropPgFormDoc.h : interface of the CCPropPgFormDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHEETINVIEWDOC_H__96C3F26E_AD99_11D3_AB38_005004CF7A2A__INCLUDED_)
#define AFX_SHEETINVIEWDOC_H__96C3F26E_AD99_11D3_AB38_005004CF7A2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPropPgFormDoc : public CDocument
{
protected: // create from serialization only
	CPropPgFormDoc();
	DECLARE_DYNCREATE(CPropPgFormDoc)

public:
	virtual ~CPropPgFormDoc();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropPgFormDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPropPgFormDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHEETINVIEWDOC_H__96C3F26E_AD99_11D3_AB38_005004CF7A2A__INCLUDED_)
