#pragma once


// CMeshTypeChange dialog

class CMeshTypeChange : public CDialog
{
	DECLARE_DYNAMIC(CMeshTypeChange)

public:
	CMeshTypeChange(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMeshTypeChange();

// Dialog Data
	enum { IDD = IDD_MESHEXTENSION_DIALOG };
   	long	m_numberofprismlayers;
	double	m_thicknessofprismlayer;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
