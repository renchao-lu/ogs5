#pragma once

#include "ListCtrl.h"
#include "EditCell.h"
#include "afxcmn.h"
#include "CApplication.h"
#include "CAreas.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "afxwin.h"

#include "rf_mmp_new.h"
#include ".\gs_mat_mp_dataedit.h"

class CMATGroupEditorDataEdit;
using ::CMATGroupEditorDataEdit;

// CMATGroupEditor dialog
class CMATGroupEditor : public CDialog
{
	DECLARE_DYNAMIC(CMATGroupEditor)
public:
	CMATGroupEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMATGroupEditor();
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonCreate();
    afx_msg void OnButtonNewMatGroup();
    afx_msg void OnBnClickedButtonWriteMP();
    afx_msg void OnBnClickedCreateFields();

// Dialog Data
	enum { IDD = IDD_MAT_GROUP_EDITOR };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:
    CString m_strMATDbName;
    int m_iSelectedMATGroup;
    CString m_strGEOName;
    CString m_strGEOTypeName;
	int Count;
	int MarkedRow;
    CString m_fileopen;
    CString m_newgroup;

    CMediumProperties* m_mmp;
public:
    CString m_strFileNameBase;
    afx_msg void OnBnClickedButtonWriteTec();
    //JG ---- Handling Excel-Files ----
    CApplication oExcel;
    CWorkbooks oBooks;    
    CWorkbook oBook;
    CEdit m_sfilename;
    CEdit m_snewname;
    CComboBox m_combo_typenames;
    afx_msg void OnFileopen();
    afx_msg void OnBnClickedGetvalues();
    //CString m_fileopen;
    CString m_strDBTypeName;
    //afx_msg void OnBnClickedButtonMATUpdate(); //OK
    CListCtrl m_LC_MMP;
    int m_iSiteSelected;
    //afx_msg void OnNMClickListcontrolMmp(NMHDR *pNMHDR, LRESULT *pResult);
    int m_iSelectedMMPGroup;
    afx_msg void OnBnClickedGridRemoveButton();
    afx_msg void OnBnClickedMATUpdateButton();

    bool dataedit_pre;
    bool dataremove;
    //void Excel_typenames2ComboBox(void);
    void GetTypenamefromComboBox(void);
    afx_msg void OnCbnSelchangeComboMATNames();
    CComboBox m_CB_MMP_PROPERTIES;
    //void matrix2MMP(void);
    afx_msg void OnLvnItemchangedListcontrolMMP(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCbnSelchangeComboMMPProperties();
    CString m_strMMPPropertyName; //OK
    CListBox m_LB_GEO;
    CComboBox m_CB_GEO_TYPE;
    afx_msg void OnBnClickedButtonGeoMat();
    CString m_strMATName;
    afx_msg void OnLbnSelchangeListMatGeo();
    //{{AFX_MSG(CMATGroupEditor)
    afx_msg void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
	//}}AFX_MSG
    int m_iSelectedGEO;
    CListBox m_LB_GEO_TYPE;
    afx_msg void OnCbnSelchangeComboMatGeoType();
    afx_msg void OnBnClickedButtonMatGeoClear();
    afx_msg void OnLbnSelchangeListMatGeoType();
    void FillTable(); //OK/JG
    CString m_str;//for function FillTable()
    void Excel_typenames2Combo(void);
    void CSV_typenames2Combo(string);
    string readCSV_type_name(string, string *);
	afx_msg void OnClick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclick_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocus_LC_MMP(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEnSetfocusNewname();
    void UpdateMMP(void);
    afx_msg void OnCbnSelchangeComboMatLay();
    // Buffer
    CMATGroupEditorDataEdit *dataeditdirect;
    int mat_type; //OKJG
};
