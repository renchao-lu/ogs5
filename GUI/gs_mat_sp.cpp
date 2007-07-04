// MAT_Mech_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_mat_sp.h"
#include "rf_msp_new.h"
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
	    WW  01-2004
	    WW  07-2007
*/


MAT_Mech_dlg::MAT_Mech_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(MAT_Mech_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(MAT_Mech_dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    plastic_data = NULL;
    creep_data = NULL;
    creep_model = 1;
    m_msp=NULL;
}

MAT_Mech_dlg::~MAT_Mech_dlg()
{
	//{{AFX_DATA_INIT(MAT_Mech_dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	delete [] plastic_data;
	if(creep_data) delete [] creep_data;
}

void MAT_Mech_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MAT_Mech_dlg)
	DDX_Control(pDX, IDC_COMBO_PLASTICITY, Combo_plastity);
	DDX_Control(pDX, IDC_MAT_MECH_GRID, m_grid);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COMBO_MAT_GROUP, mat_group);
	DDX_Control(pDX, IDC_COMCREEP, combox_creep);
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
	ON_CBN_SELCHANGE(IDC_COMBO_MAT_GROUP, OnCbnSelchangeComboMatGroup)
	ON_BN_CLICKED(ID_MSP_UPDATE, OnBnClickedMspUpdate)
	ON_BN_CLICKED(ID_MSP_NEW, OnBnClickedMspNew)
	ON_CBN_SELCHANGE(IDC_COMCREEP, OnCbnSelchangeComcreep)
	ON_BN_CLICKED(IDC_CREEP, OnBnClickedCreep)
	ON_CBN_SELCHANGE(IDC_CREEP1, OnCbnSelchangeCreep1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MAT_Mech_dlg message handlers
BOOL MAT_Mech_dlg::OnInitDialog()
{
    NumRows = 2;
    //
	CDialog::OnInitDialog();
    mat_group.SetCurSel(0);
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
	for(i=0; i<4; i++)
       Type[i] = false;
    plastic_data = new double[23];
    //
    if(msp_vector.size())
    {
       m_msp = msp_vector[0];
       Thermal[0] = m_msp->Heat_Conductivity();
       Thermal[1] = m_msp->Heat_Capacity();
       if(fabs(Thermal[0])+fabs(Thermal[1])>DBL_MIN)
         Type[0] = true;
       Elast[0] = m_msp->Youngs_Modulus();
       Elast[1] = m_msp->Poisson_Ratio();
       Elast[2] = m_msp->Thermal_Expansion();
       Type[1] = true; 
       PlastModel = m_msp->Plastictity()-1;   
       if(m_msp->Plastictity()==1)
       {
         Combo_plastity.SetCurSel(0);
         Type[2] = true;
         for(i=0; i<5; i++)	
           plastic_data[i] = m_msp->GetPlasticParameter(i);
       }
       else if(m_msp->Plastictity()==2)
       {
         Type[2] = true;
         Combo_plastity.SetCurSel(1);
         for(i=0; i<23; i++)	
           plastic_data[i] = m_msp->GetPlasticParameter(i);
       }
       else if(m_msp->Plastictity()==3)
       {
         Type[2] = true;
         Combo_plastity.SetCurSel(2);
         for(i=0; i<10; i++)	
           plastic_data[i] = m_msp->GetPlasticParameter(i);
       }
    }
    else
    {       
       for(i=0; i<2; i++)
         Thermal[i]=0.0;
       for(i=0; i<3; i++)
         Elast[i]=0.0;
       for(i=0; i<23; i++)
	     plastic_data[i]=0.0;
    }
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

        // Plastic
		if(Type[2]&&TheRow>=RowShift)
		{			
          plastic_data[TheRow-RowShift]=Val;
          switch(PlastModel)
		  {
             case 0:
                RowShift += 5;
		        break;
             case 1:
                RowShift += 23;
    	        break;
             case 2:
                RowShift += 10;
		        break;
		   }  		
         }	
         else if(!Type[2])	   
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
void MAT_Mech_dlg::OnBnClickedCreep()
{
	if(!Type[3]) 
		Type[3]= true;
	else Type[3]= false;
    Refresh();		
}
void MAT_Mech_dlg::Refresh() 
{
    int i=0;
    int num_items = 4; 
    if(Type[2]) //Plastic
	{
       switch(PlastModel)
       {
	      case 0:
             ParaNum[2]=5;
             Combo_plastity.SetCurSel(0);
			 break;
	      case 1:
             ParaNum[2]=10;
             Combo_plastity.SetCurSel(1);
			 break;
	      case 2:
             ParaNum[2]=23;
             Combo_plastity.SetCurSel(2);
			 break;			  
	   }		   
	}
    if(Type[3]) // Creep
    {
       if(!creep_data) 
          creep_data = new double[3];
       for(i=0; i<3; i++)
         creep_data[i] = 0.0;
       switch(creep_model)
       {
	      case 1:
             ParaNum[3]=2;
             combox_creep.SetCurSel(0);
			 break;
	      case 2:
             ParaNum[3]=3;
             combox_creep.SetCurSel(1);
			 break;
	   }		          
    }		
    //
    NumRows = 0;
    for(int i=0; i<num_items; i++)
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
    // Plastic
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
    else
      Combo_plastity.SetCurSel(-1);
    // Plastic
    if(Type[3])
	{
       switch(creep_model)
       {
	      case 1:
             AddCRP_Norton();
			 break;
	      case 2:
             AddCRP_BGRa();
			 break;
	   }		   
	}
    else
      combox_creep.SetCurSel(-1);
    //     		
    m_grid.RedrawWindow();
}


void MAT_Mech_dlg::AddThermal() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Thermal capacity"), Thermal[0], _T("J/kg.°C")));
   m_Data.Add(Mat_Mech_Grid(_T("Thermal conductivity"), Thermal[1], _T("W/m.°C")));
}

void MAT_Mech_dlg::AddElasticity() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Young's modulus"), Elast[0], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Poisson ratio"), Elast[1], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Thermal expansion"), Elast[2], _T("1/°C")));
}

void MAT_Mech_dlg::AddPL_DP() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Initial cohesion"), plastic_data[0], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Plastic hardening"), plastic_data[1], _T("Pa")));
   m_Data.Add(Mat_Mech_Grid(_T("Frictional angle"), plastic_data[2], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Dilatancy angle"), plastic_data[3], _T("--")));
   m_Data.Add(Mat_Mech_Grid(_T("Localized hardening modulus"), plastic_data[4], _T("Pa/m")));
}
void MAT_Mech_dlg::AddPL_CM() 
{
    m_Data.Add(Mat_Mech_Grid(_T("Slope of the critical line"), plastic_data[0], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Virgin compression index"), plastic_data[1], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Swelling index"), plastic_data[2], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Preconsolidation pressure"), plastic_data[3], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial void ratio"), plastic_data[4], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("OCR"), plastic_data[5], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_xx"), plastic_data[6], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_yy"), plastic_data[7], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_zz"), plastic_data[8], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Mimimum stress"), plastic_data[9], _T("Pa")));
}
//
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

    m_Data.Add(Mat_Mech_Grid(_T("alpha_0"), plastic_data[0], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("beta_0"), plastic_data[1], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("delta_0"), plastic_data[2], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("epsilon_0"), plastic_data[3], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("kappa_0"), plastic_data[4], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("gamma_0"), plastic_data[5], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("m_0"), plastic_data[6], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("alpha_1"), plastic_data[7], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("beta_1"), plastic_data[8], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("delta_1"), plastic_data[9], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("epsilon_1"), plastic_data[10], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("kappa_1"), plastic_data[11], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("gamma_1"), plastic_data[12], _T("--")));
    m_Data.Add(Mat_Mech_Grid(_T("m_1"), plastic_data[13], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("psi_1"), plastic_data[14], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("psi_2"), plastic_data[15], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("Ch"), plastic_data[16], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Cd"), plastic_data[17], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("br"), plastic_data[18], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("mr"), plastic_data[19], _T("Pa")));

    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_xx"), plastic_data[20], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_yy"), plastic_data[21], _T("Pa")));
    m_Data.Add(Mat_Mech_Grid(_T("Initial stress_zz"), plastic_data[22], _T("Pa")));


}
// Creep 04.07.2007
void MAT_Mech_dlg::AddCRP_Norton() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Creep ratio"), creep_data[0], _T("Pa/s")));
   m_Data.Add(Mat_Mech_Grid(_T("Exponential index"), creep_data[1], _T("--")));
}
void MAT_Mech_dlg::AddCRP_BGRa() 
{
   m_Data.Add(Mat_Mech_Grid(_T("Creep ratio"), creep_data[0], _T("Pa/s")));
   m_Data.Add(Mat_Mech_Grid(_T("Activation energy"), creep_data[1], _T("kJ/mol")));
   m_Data.Add(Mat_Mech_Grid(_T("Exponential index"), creep_data[1], _T("--")));
}

void MAT_Mech_dlg::OnSelchangeComboPlasticity() 
{
    ///Combobox Plasticity
	PlastModel =Combo_plastity.GetCurSel();

    Refresh();
}
void MAT_Mech_dlg::OnCbnSelchangeComcreep()
{
    ///Combobox Plasticity
	creep_model =combox_creep.GetCurSel();
    Refresh();
}



void MAT_Mech_dlg::OnCbnSelchangeComboMatGroup()
{
  
}

void MAT_Mech_dlg::OnBnClickedMspUpdate()
{
	// TODO: Add your control notification handler code here
}

void MAT_Mech_dlg::OnBnClickedMspNew()
{
	// TODO: Add your control notification handler code here
}





void MAT_Mech_dlg::OnCbnSelchangeCreep1()
{
	creep_model =combox_creep.GetCurSel();
    Refresh();
}
