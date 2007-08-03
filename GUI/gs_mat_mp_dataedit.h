#pragma once

#include "ListCtrl.h"
#include "EditCell.h"
#include "afxcmn.h"

// EXCEL
//#include "CApplication.h"
//#include "CAreas.h"
//#include "CRange.h"
//#include "CWorkbook.h"
//#include "CWorkbooks.h"
//#include "CWorksheet.h"
//#include "CWorksheets.h"
#include ".\gs_mat_mp.h"//Excel-Includes

extern vector<CString> type_name_vector;
extern vector<CString> key_word_vector;

#include "matrix_class.h"
#include "afxwin.h"
using Math_Group::Matrix;
extern Math_Group::Matrix* p_matrix;

//OK namespace SolidProp{ class CSolidProperties; };
//OK using SolidProp::CSolidProperties;
class CMATGroupEditorDataEdit : public CDialog
{
	DECLARE_DYNAMIC(CMATGroupEditorDataEdit)

public:
	CMATGroupEditorDataEdit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMATGroupEditorDataEdit();
    virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_MAT_GROUP_EDITOR_DATAEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:

public:
    // ---- Handling Excel-Files ----
    CApplication oExcel;
    CWorkbooks oBooks;    
    CWorkbook oBook;
    int pvmCols, pvmRows;
    afx_msg void OnBnClickedOK();
    CString m_fileopen;
    CString m_strDBTypeNamePre;
    CMyListCtrl m_listctrldata;
    CEdit m_typenamedispl;
    CString m_tndisplstr;
    //void Excel2matrix(void);//JG matrix wird im moment nicht verwendet
    //void matrix2ListCtrl(void);//JG matrix wird im moment nicht verwendet
    //void ListCtrl2matrix(void);//JG matrix wird im moment nicht verwendet
    //void ExcelDirect2matrix(void);//JG matrix wird im moment nicht verwendet
    void EmptytnkwVectors(void);
    void SafeArray2ListCtrl(void);
    void ListCtrl2SafeArray(void);
    void ExcelDirect2SafeArray(void);
    void CSVtextDirect2SafeArray(void);
    void CSVtext2ListCtrl(void);
    void CSV_typenames2typenamevector(string);
    string readCSV_type_name(string, string *);//
    void CSV_keywords2keywordvector(string);
    string readCSV_key_word(string, string *);
    void StandardKeywords(void);
    void MMP2UpdateListCtrl(void);
    void Keywords2NewListCtrl(void);
    void SafeArray2MMP(CMediumProperties* m_mmp0=NULL);
    void SafeArray2MSP(CSolidProperties*); //OK
    CMediumProperties* m_mmp;
    CSolidProperties* m_msp;
    void MSP2UpdateListCtrl();
    void MSPStandardKeywords();
};

