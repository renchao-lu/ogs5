// MAT_Mech_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_mat_sp.h"
#include ".\gs_mat_sp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MAT_Mech_dlg dialog
/*
    Dialog for mechanical material properties.
	Designed and programmed: WW
	Last modification: WW  01-2004
*/


MAT_Mech_dlg::MAT_Mech_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(MAT_Mech_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(MAT_Mech_dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MAT_Mech_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MAT_Mech_dlg)
	DDX_Control(pDX, IDC_COMBO_PLASTICITY, Combo_plastity);
    DDX_Control(pDX, IDC_MAT_MECH_GRID, m_grid);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MAT_Mech_dlg, CDialog)
	//{{AFX_MSG_MAP(MAT_Mech_dlg)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_MAT_MECH_GRID, OnGridGetDispInfo)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_MAT_MECH_GRID, OnGridSetDispInfo)
	ON_BN_CLICKED(IDC_CHECK_THERMAL, OnCheckThermal)
	ON_BN_CLICKED(IDC_CHECK_ELASTICITY, OnCheckElasticity)
	ON_BN_CLICKED(IDC_CHECK_PLASTICITY, OnCheckPlasticity)
	ON_CBN_SELCHANGE(IDC_COMBO_PLASTICITY, OnSelchangeComboPlasticity)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MAT_Mech_dlg message handlers
BOOL MAT_Mech_dlg::OnInitDialog()
{
    NumRows = 2;

	CDialog::OnInitDialog();

	Combo_plastity.SetCurSel(0);

	/////// Grid
	//  ---------------  Required variables  -----------------
	CGridColumn *pColumn;
	CGridHeaderSections *pSections;
	//CGridHeaderSection *pSection;
	CGridHeaderSection *pUpperSection;

	m_grid.SetRowHeight(25);

	//  -----------------  Let's add some columns  --------------  
	m_grid.AddColumn(_T(""), 128, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 128, LVCFMT_LEFT);
	m_grid.AddColumn(_T(""), 64, LVCFMT_LEFT);
	

	//  ---------------  Set additional column properties  ----------------  

	pColumn = m_grid.GetColumn(0);
	pColumn->SetReadOnly(TRUE);
	//pColumn->SetWordWrap(TRUE);

	pColumn = m_grid.GetColumn(2);
	pColumn->SetReadOnly(TRUE);

	//  ---------------  Let's put the grid header into shape  ------------
	pSections = m_grid.GetHeader()->GetSections();
	pUpperSection = pSections->GetSection(0);
	pUpperSection->SetCaption(_T("Parameter"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(1);
	pUpperSection->SetCaption(_T("Value"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	pUpperSection = pSections->GetSection(2);
	pUpperSection->SetCaption(_T("Unit"));
	pUpperSection->SetAlignment(LVCFMT_CENTER);
	m_grid.GetHeader()->SynchronizeSections();

	//  --------------  Some additional initializations...  ------
	/*
	m_grid.SetAllowEdit();
	m_grid.SetRowCount(NumRows);

	m_Data.Add(Mat_Mech_Grid(_T("Heat capacity"), 0.0, _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Hest conductivity"), 0.0, _T("--")));
    */

    /* Default values */
	PlastModel = 0;

    ParaNum[0]=2; //Number of thermal parameters
    ParaNum[1]=3; //Number of elasticity parameters
    ParaNum[2]=4; //Number of plasticity parameters

	int i;
	for(i=0; i<3; i++)
       Type[i] = false;

	for(i=0; i<2; i++)
       Thermal[i]=0.0;
	for(i=0; i<3; i++)
       Elast[i]=0.0;
	for(i=0; i<4; i++)	
       DruckP[i]=0.0;
	for(i=0; i<9; i++)
	   CamC[i]=0.0;
	for(i=0; i<23; i++)
	   RotH[i]=0.0;
   
	return TRUE;
}
//  ---
void MAT_Mech_dlg::OnGridGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    char buffer[128];
	pResult = pResult;

	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;

	if (pDispInfo->item.mask & LVIF_TEXT)  {
		switch (pDispInfo->item.nColumn)  {
		case 0:		//  Parameter
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strParaName;
            break;
		case 1:		//  Value
            _gcvt(m_Data[pDispInfo->item.nRow].ParaVal, 12, buffer );
			pDispInfo->item.strText = buffer; 
			break;
		case 2:		//  Unit
			pDispInfo->item.strText 
				= m_Data[pDispInfo->item.nRow].m_strUnitName;
			break;
		default:
			break;
		}
	}

}
//  ---
void MAT_Mech_dlg::OnGridSetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	VG_DISPINFO *pDispInfo = (VG_DISPINFO *)pNMHDR;

    int TheRow=0; 
	int RowShift = 0;
	double Val=0.0;

	switch (pDispInfo->item.nColumn)  {
	case 0:		//  
        break;
	case 1:		//  Value
		TheRow=pDispInfo->item.nRow;
		Val = strtod(pDispInfo->item.strText, NULL);
		m_Data[pDispInfo->item.nRow].ParaVal = Val;

		RowShift=0;
        if(Type[0]&&Type[1])
        {
           RowShift=5;
           if(TheRow<2)
               Thermal[TheRow] = Val;
           if(TheRow>=2&&TheRow<5)
               Elast[TheRow-2] = Val;
		}
        else if(!Type[0]&&Type[1])
        {
           RowShift=3;
           if(TheRow<3)
               Elast[TheRow] = Val;
		}
        else if(Type[0]&&!Type[1])
        {
            RowShift=2;
            if(TheRow<2)
               Thermal[TheRow] = Val;
		}
        else if(!Type[0]&&!Type[1])
            RowShift=0;


		if(Type[2]&&TheRow>=RowShift)
		{			
          switch(PlastModel)
		  {
             case 0:
                DruckP[TheRow-RowShift]=Val;
		        break;
             case 1:
                CamC[TheRow-RowShift]=Val;
    	        break;
             case 2:
                RotH[TheRow-RowShift]=Val;
		        break;
		   }  
		}		   
		break;
	default:
		break;
	}
	
 
	*pResult = 0;
}

void MAT_Mech_dlg::OnCheckThermal() 
{
	if(!Type[0]) 
		Type[0]= true;
	else Type[0]= false;

    Refresh();
}

void MAT_Mech_dlg::OnCheckElasticity() 
{
	if(!Type[1]) 
		Type[1]= true;
	else Type[1]= false;

    Refresh();
}

void MAT_Mech_dlg::OnCheckPlasticity() 
{
	if(!Type[2]) 
		Type[2]= true;
	else Type[2]= false;

    Refresh();
		
}

void MAT_Mech_dlg::Refresh() 
{
    if(Type[2])
	{
       switch(PlastModel)
       {
	      case 0:
             ParaNum[2]=4;
			 break;
	      case 1:
             ParaNum[2]=9;
			 break;
	      case 2:
             ParaNum[2]=23;
			 break;			  
	   }		   
	}		

    NumRows = 0;
    for(int i=0; i<3; i++)
      NumRows += ParaNum[i]*(int)Type[i]; 
	
  	m_grid.SetAllowEdit();
    m_grid.SetRowCount(NumRows);
    m_Data.RemoveAll();

	if(Type[0]&&Type[1])
    { 
        AddThermal();
		AddElasticity();
	}
	else if(Type[0]&&!Type[1])
        AddThermal();
	else if(!Type[0]&&Type[1])
		AddElasticity();

    if(Type[2])
	{
       switch(PlastModel)
       {
	      case 0:
             AddPL_DP();
			 break;
	      case 1:
             AddPL_CM();
			 break;
	      case 2:
             AddPL_RH();
			 break;			  
	   }		   
	}		

    m_grid.RedrawWindow();
}


void MAT_Mech_dlg::AddThermal() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Heat capacity"), Thermal[0], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Hest conductivity"), Thermal[1], _T("--")));
}

void MAT_Mech_dlg::AddElasticity() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Young's modulus"), Elast[0], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Poisson ratio"), Elast[1], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Thermal expansion"), Elast[2], _T("--")));
}

void MAT_Mech_dlg::AddPL_DP() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Initial cohesion"), DruckP[0], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Plastic hardening"), DruckP[1], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Frictional angle"), DruckP[2], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Dilatancy angle"), DruckP[3], _T("--")));
}
void MAT_Mech_dlg::AddPL_CM() 
{
    m_Data.Add(Mat_Mech_Grid(_T("Slope of the critical line"), CamC[0], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Virgin compression index"), CamC[1], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Swelling index"), CamC[2], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Preconsolidation pressure"), CamC[3], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial void ratio"), CamC[4], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("OCR"), CamC[5], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_xx"), CamC[6], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_yy"), CamC[7], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_zz"), CamC[8], _T("Pa")));
}

void MAT_Mech_dlg::AddPL_RH() 
{
    /*
       Material parameters for EHLERS_WEIMAR
				   i: parameter
				   0: alpha0
				   1: beta0
				   2: delta0
				   3: epsilon0 
				   4: kappa0 
				   5: gamma0
				   6: m0

				   7: alpha1
				   8: beta1
				   9: delta1
				  10: epsilon1 
				  11: kappa1
				  12: gamma1
				  13: m1
                   
				  14: Psi1
				  15: Psi2

                  16: Ch
				  17: Cd
				  18: br
				  19: mr

				  20: Initial stress_xx 
				  21: Initial stress_yy 
				  22: Initial stress_zz 
    */

    m_Data.Add(Mat_Mech_Grid(_T("alpha_0"), RotH[0], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("beta_0"), RotH[1], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("delta_0"), RotH[2], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("epsilon_0"), RotH[3], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("kappa_0"), RotH[4], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("gamma_0"), RotH[5], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("m_0"), RotH[6], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("alpha_1"), RotH[7], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("beta_1"), RotH[8], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("delta_1"), RotH[9], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("epsilon_1"), RotH[10], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("kappa_1"), RotH[11], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("gamma_1"), RotH[12], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("m_1"), RotH[13], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("psi_1"), RotH[14], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("psi_2"), RotH[15], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("Ch"), RotH[16], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Cd"), RotH[17], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("br"), RotH[18], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("mr"), RotH[19], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_xx"), RotH[20], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_yy"), RotH[21], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_zz"), RotH[22], _T("Pa")));


}


void MAT_Mech_dlg::OnSelchangeComboPlasticity() 
{
    ///Combobox Plasticity
	PlastModel =Combo_plastity.GetCurSel();

    Refresh();
}


