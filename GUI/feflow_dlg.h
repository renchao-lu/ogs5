#pragma once


// CDialogFEFLOW dialog

class CDialogFEFLOW : public CDialog
{
	DECLARE_DYNAMIC(CDialogFEFLOW)

public:
	CDialogFEFLOW(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogFEFLOW();

// Dialog Data
	enum { IDD = IDD_FEFLOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedReadNodes();
    afx_msg void OnBnClickedReadElements();
    afx_msg void OnBnClickedReadBoundaryConditions();
    afx_msg void OnBnClickedReadSourceSinkTerms();
    afx_msg void OnBnClickedCreateNOD();
    afx_msg void OnBnClickedCreateELE();
    afx_msg void OnBnClickedCreateBC();
    afx_msg void OnBnClickedCreateST();

private:
  CFEMesh* m_msh;

};
