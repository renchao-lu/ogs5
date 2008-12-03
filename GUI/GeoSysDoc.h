// GeoSysDoc.h : interface of the CGeoSysDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOSYSDOC_H__59CED134_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_GEOSYSDOC_H__59CED134_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <string>
#include <vector>
#include <list>
using namespace std;
#include "makros.h"
#include "nodes.h"
#include "rf_tim_new.h"
//#include "test.h"
class CGeoSysDoc : public CDocument
{
public: // create from serialization only
	CGeoSysDoc();
	DECLARE_DYNCREATE(CGeoSysDoc)
    void GSPReadWIN(CArchive& ar);
    void GSPWriteWIN(CArchive& ar);
// Attributes
public:
    CString m_strNamePolylineSelected;
    // Files
	CString m_strGSPFilePathBaseExt;//with extension
    CString m_strGSPFilePathBase;//without extension
    CString m_strGSPFilePath; // inkl. "/"
	CString m_strGSPFileBase; // only name
    CString m_strGSPFileExt; //with point ".rfi"
    // Flags
    bool m_bDataGEO;
    bool m_bDataMSH;
    bool m_bDataFEM;
    bool m_bDataRFO;
    bool m_bDataPCS;
    BOOL m_bReady2Run;
    // Bitmap
    CBitmap m_bmpBitmap;
    double m_dXmin,m_dXmax;
    double m_dYmin,m_dYmax;
    int m_iFluidPhase;
    CString m_strQuantityName;
    FEMNodesElements *m_nodes_elements;
    // TIM
    CTimeDiscretization *m_doc_tim;
    CString m_strPCSTypeName; //OK
// Operations
public:
    // Implementation CC 04/2004
protected:
	CSize           m_sizeDoc;
public:
    CSize GetDocSize() { return CSize(1280,1024); }
   // CSize GetDocSize() { return CDC::GetWindowExt( ); }
    
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeoSysDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
    
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeoSysDoc();
    virtual void InitDocument();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    // Files
    CString m_strFileName;
    CString m_strFileNameRFD;
    CString m_strFileNameRFE;
    void WriteRFD(char *);

// Generated message map functions
protected:
	//{{AFX_MSG(CGeoSysDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnAddGEO();    
    afx_msg void OnAddMSH();    
    afx_msg void OnAddFEM();

    afx_msg void OnFileSaveAs();
    afx_msg void OnFileSave();
    afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
    afx_msg void OnUpdateOpenMSH(CCmdUI *pCmdUI);
    afx_msg void OnUpdateOpenFEM(CCmdUI *pCmdUI);
	afx_msg void GSPRemoveMemberFromProject(CString MemberType);
    afx_msg void OnSaveGEO();
	afx_msg void OnSaveMSH();
	afx_msg void OnSaveFEM();
	afx_msg void OnExportTecFile();
    afx_msg void OnSimulatorForward();
    afx_msg void OnImportASC();
	afx_msg void OnRemoveGEO();
	afx_msg void OnRemoveMSH();
    afx_msg void OnSaveAsGEO();
	afx_msg void OnSaveAsMSH();
	afx_msg void OnSaveAsFEM();
	afx_msg void OnImportSHP();
    afx_msg void OnImportRFO();
    afx_msg void OnImportBMP();
    afx_msg void OnImportEXCEL_PNT(); //OK
    afx_msg void OnImportEXCEL_FCT(); //OK
    afx_msg void OnRemoveFEM();
    afx_msg void OnUpdateAddGEO(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAddMSH(CCmdUI *pCmdUI);
    afx_msg void OnUpdateAddFEM(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRemoveGEO(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRemoveMSH(CCmdUI *pCmdUI);
    afx_msg void OnUpdateRemoveFEM(CCmdUI *pCmdUI);
    afx_msg void OnSimulatorCheckStatus();
    afx_msg void OnUpdateSimulatorForward(CCmdUI *pCmdUI);
    afx_msg void OnImportGMS();
    afx_msg void OnOpenOGL();
	afx_msg void OnImportSHPNew();
	afx_msg void OnImportFLAC(); //OK
    afx_msg void OnImportFLACMesh(); //MR
    afx_msg void OnImportFEFLOW(); //OK
};

/////////////////////////////////////////////////////////////////////////////

extern string GetProjectFileName();
extern void MSHOpen(string file_name_base);
extern void FEMOpen(string file_name_base);
extern bool m_SHP;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEOSYS_H__59CED134_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
