#pragma once


// CStructured_Mesh dialog

class CStructured_Mesh : public CDialog
{
	DECLARE_DYNAMIC(CStructured_Mesh)

public:
	CStructured_Mesh(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStructured_Mesh();

// Dialog Data
	enum { IDD = IDD_STRUCTURED_MESH_DIALOG };
   	double m_edit_x_min;
    double m_edit_y_min;
    double m_edit_x_max;
    double m_edit_y_max;
    long   m_delta_x, m_delta_y;
    BOOL m_do_structured_triangle_mesh;
    BOOL m_do_structured_rectangle_mesh;



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
};
