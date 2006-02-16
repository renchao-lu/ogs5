// Cgs_pcs_oglcontrol dialog

//#includes:


class Cgs_pcs_oglcontrol : public CDialog
{
	DECLARE_DYNAMIC(Cgs_pcs_oglcontrol)

public:
	Cgs_pcs_oglcontrol(CWnd* pParent = NULL);   // standard constructor
	virtual ~Cgs_pcs_oglcontrol();

// Dialog Data
	enum { IDD = IDD_PCS_DISPLAY_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
    double m_pcs_min;
    double m_pcs_max;
    CString m_pcs_name;
public: 
    void OnControlPanel(); 
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedGetPcsMinmax();
};
