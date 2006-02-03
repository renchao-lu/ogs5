// PTValue.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "PTValue.h"

// PTValue dialog

IMPLEMENT_DYNAMIC(PTValue, CDialog)
PTValue::PTValue(CWnd* pParent /*=NULL*/)
	: CDialog(PTValue::IDD, pParent)
    , m_NumOfParticles(0) 
{
    m_msh = NULL;
    fem = NULL;
    m_pcs = NULL;
    m_ele = NULL;
}

PTValue::~PTValue()
{
   delete fem;
}

void PTValue::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_NUMOFPARTICLES, m_NumOfParticles);
    DDV_MinMaxInt(pDX, m_NumOfParticles, 0, 1000000);
    DDX_Control(pDX, IDC_PICKEDINFO, m_PickedInfo);
}


BEGIN_MESSAGE_MAP(PTValue, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()




// PTValue message handlers


BOOL PTValue::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // Update the number of the selected elements for assigning PT values
    CString howMany;
    howMany.Format(_T("%d elements are selected for particles."), theApp.hitsElementTotal);
   
    // Then, change the text accordingly
    m_PickedInfo.SetWindowText(howMany);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void PTValue::OnBnClickedOk()
{
    UpdateData(TRUE);

    int naturalNum = 0, naturalNumPlusOne = 0;
   
    // Check the element type first
    m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
    // Allocate memory for the new version
    fem = new CElement(m_msh->GetCoordinateFlag());  
    
    m_msh->PT->numOfParticles = m_NumOfParticles;
    // Allocate memory for number of particle objects
    m_msh->PT->CreateParticles(m_msh->PT->numOfParticles);
    
    // Get the natural number obtained by dividing the number of elements
    // picked by the number of particles

    // I am starting with the number and the indeces of the elements
    // These can be configured later on in the file like .pct 
    // I need to discuss this with Olaf.
    
    // This may look weired. But it makes sense. 
    if( (m_msh->PT->numOfParticles / theApp.hitsElementTotal) * theApp.hitsElementTotal 
        < m_msh->PT->numOfParticles )
    {
        naturalNum = m_msh->PT->numOfParticles / theApp.hitsElementTotal; 
        naturalNumPlusOne = naturalNum + 1;
    }
    else
        naturalNumPlusOne = naturalNum = m_msh->PT->numOfParticles / theApp.hitsElementTotal; 


    int count = 0;

    // Loop over the number of the picked elements
    for(int i=0; i<theApp.hitsElementTotal; ++i)
    {
        m_ele = m_msh->ele_vector[theApp.elementPickedTotal[i]];
        if(i != (theApp.hitsElementTotal -1) )
        {
            // Assign N number of particles in this element;
            for(int j=0; j<naturalNumPlusOne; ++j)
            {
                AssignAParticleToTheElement(count, m_ele);
                ++count;
            }
        }
        else
        {
            // Assign N number of particles in this element;  
            for(int j=0; j<naturalNum; ++j)
            {
                AssignAParticleToTheElement(count, m_ele);   
                ++count;
            } 
        }     
    }
    
    OnOK();
}

void PTValue::AssignAParticleToTheElement(int no, CElem* m_ele)
{

    double unit[4];

    fem->ConfigElement(m_ele);
    fem->ConfigNumerics(m_ele->GetElementType());
   
    // Assign the index of the element
    m_msh->PT->X[no].Past.elementIndex = m_msh->PT->X[no].Now.elementIndex = m_ele->GetIndex(); 

    // Check the element
    /*LINES = 1*/ 
    if (m_ele->GetElementType() == 1)
	{
        unit[0] = m_msh->PT->randomMinusOneToOne();
        unit[1] = 0.0;
        unit[2] = 0.0;
        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }
    /*RECTANGLES = 2*/ 
	if (m_ele->GetElementType() == 2)
	{
        unit[0] = m_msh->PT->randomMinusOneToOne();
        unit[1] = m_msh->PT->randomMinusOneToOne();
        unit[2] = 0.0;
        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }
    /*HEXAHEDRA = 3*/ 
	if (m_ele->GetElementType() == 3)
	{
        unit[0] = m_msh->PT->randomMinusOneToOne();
        unit[1] = m_msh->PT->randomMinusOneToOne();
        unit[2] = m_msh->PT->randomMinusOneToOne();
        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }
    /*TRIANGLES = 4*/ 
	if (m_ele->GetElementType() == 4)
	{
        do{
            unit[0] = m_msh->PT->randomZeroToOne();
            unit[1] = m_msh->PT->randomZeroToOne();
            unit[2] = 0.0;
        }while(unit[0] + unit[1] > 1.0);

        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }
    /*TETRAHEDRAS = 5*/ 
	if (m_ele->GetElementType() == 5)
	{
        do{
           unit[0] = m_msh->PT->randomZeroToOne();
           unit[1] = m_msh->PT->randomZeroToOne();
           unit[2] = m_msh->PT->randomZeroToOne();
        }while(unit[0] + unit[1] + unit[2] > 1.0);

        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }
    /*PRISMS = 6*/ 
	if (m_ele->GetElementType() == 6)
	{
        do{
            unit[0] = m_msh->PT->randomZeroToOne();
            unit[1] = m_msh->PT->randomZeroToOne();
         }while(unit[0] + unit[1] > 1.0);
        unit[2] = m_msh->PT->randomMinusOneToOne();
        
        fem->setUnitCoordinates(unit);
        fem->ComputeShapefct(1);
        fem->RealCoordinates(unit);

        // Assign the number to the particle
        m_msh->PT->X[no].Past.x = m_msh->PT->X[no].Now.x = unit[0];
        m_msh->PT->X[no].Past.y = m_msh->PT->X[no].Now.y = unit[1];
        m_msh->PT->X[no].Past.z = m_msh->PT->X[no].Now.z = unit[2];
    }

}


