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

	// To produce a different pseudo-random series each time your program is run.
    srand((int)time(0));
}

PTValue::~PTValue()
{

   delete fem;
}

void PTValue::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_NUMOFPARTICLES, m_NumOfParticles);
	DDX_Text(pDX, IDC_PID, m_PID);
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

	if(theApp.ElementSwitch == 1)
	{
		int naturalNum = 0, naturalNumPlusOne = 0;
   
	    // Check the element type first
		m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
	    // Allocate memory for the new version
		fem = new CElement(m_msh->GetCoordinateFlag());  
    
		m_msh->PT->numOfParticles = m_NumOfParticles;

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

		// Loop over the number of the picked elements
		Trace* one;
		one = new Trace();
		// Loop over the number of the picked elements
		for(int i=0; i<theApp.hitsElementTotal; ++i)
		{
			m_ele = m_msh->ele_vector[theApp.elementPickedTotal[i]];
			if(i != (theApp.hitsElementTotal -1) )
			{
				// Assign N number of particles in this element;
				for(int j=0; j<naturalNumPlusOne; ++j)
				{
					AssignAParticleToTheElement(one, m_ele);
					one->Past.identity = one->Now.identity = m_PID;
					m_msh->PT->X.push_back(*one);
				}
			}
			else
			{
				// Assign N number of particles in this element;  
				for(int j=0; j<naturalNum; ++j)
				{
					AssignAParticleToTheElement(one, m_ele); 
					one->Past.identity = one->Now.identity = m_PID;
					m_msh->PT->X.push_back(*one);
				} 
			}     
		}
	}
	else if(theApp.PolylineSwitch == 1)
	{
		CGLPolyline* thisPolyline = NULL;
	
//		if(theApp.hitsPolylineTotal < 3)
//		{
			// Let's create the particles and solve each number of particles to be assigned to each segment.
			m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
			// Check if this is the first polyline assigned by particles.
			m_msh->PT->numOfParticles = m_NumOfParticles;
		
			int numOfParticlesInThisPolyline = 0;

			numOfParticlesInThisPolyline = m_msh->PT->numOfParticles / theApp.hitsPolylineTotal;
		
			for(int p=0; p < theApp.hitsPolylineTotal;++p)
			{		
				int count = numOfParticlesInThisPolyline*p;
				// Mount the polyline picked.
				thisPolyline = polyline_vector[theApp.polylinePickedTotal[p]];

				// Get the number of geo points and the number of line segments
				int numOfGeoPoints = (int)thisPolyline->point_vector.size();
				int numOfLineSegments = numOfGeoPoints - 1;

				double lenthOfthisPolyline = 0.0;
				double* line = NULL; int* particles = NULL;
				line = new double [numOfLineSegments] (); particles = new int [numOfLineSegments] ();
				// Let's solve for the length of each segment
				for(int i=0; i<numOfLineSegments; ++i)
				{
					double x[2], y[2], z[2];
					x[0] = thisPolyline->point_vector[i]->x; x[1] = thisPolyline->point_vector[i+1]->x;
					y[0] = thisPolyline->point_vector[i]->y; y[1] = thisPolyline->point_vector[i+1]->y;
					z[0] = thisPolyline->point_vector[i]->z; z[1] = thisPolyline->point_vector[i+1]->z;
					line[i] = sqrt((x[0]-x[1])*(x[0]-x[1])+(y[0]-y[1])*(y[0]-y[1])+(z[0]-z[1])*(z[0]-z[1]));
					lenthOfthisPolyline += line[i];
				}

				// Since I am dealing with int, the following logic is needed to have all the particles
				// properly assigned.
				for(int i=0; i<numOfLineSegments-1; ++i)
					particles[i] = numOfParticlesInThisPolyline * line[i] / lenthOfthisPolyline;
				particles[numOfLineSegments-1] = numOfParticlesInThisPolyline;
				for(int i=0; i<numOfLineSegments-1; ++i)
					particles[numOfLineSegments-1] -= particles[i];
			
				// Let's assign particles randomly segment by segment.
				// Let's have the global number of particles
				int no = count;
				for(int i=0; i<numOfLineSegments; ++i)
				{
					// Mount two points of each segment and solve vector of 10.
					double x[2], y[2], z[2], seg10[3];
					x[0] = thisPolyline->point_vector[i]->x; x[1] = thisPolyline->point_vector[i+1]->x;
					y[0] = thisPolyline->point_vector[i]->y; y[1] = thisPolyline->point_vector[i+1]->y;
					z[0] = thisPolyline->point_vector[i]->z; z[1] = thisPolyline->point_vector[i+1]->z;
					seg10[0] = x[1] - x[0]; seg10[1] = y[1] - y[0]; seg10[2] = z[1] - z[0];
					double lengthOfseq10 = sqrt(seg10[0]*seg10[0] + seg10[1]*seg10[1] + seg10[2]*seg10[2]); 
				
					Trace one;
					for(int j=0; j< particles[i]; ++j)
					{
						// Get the random distance from 0 to line[i]
						double distance = (double)(1.0*rand()/(RAND_MAX+1.0))*line[i];	
					
						// Let's solve for the position of particle No. j
						// First the constant to be multiplied by the vector 10
						double mag = distance / lengthOfseq10;
						double p[3];
						// Don't forget to translate to the 0 point of the segment
						p[0] = mag*seg10[0] + x[0]; p[1] = mag*seg10[1] + y[0]; p[2] = mag*seg10[2] + z[0];
						one.Past.x = one.Now.x = p[0];
						one.Past.y = one.Now.y = p[1];
						one.Past.z = one.Now.z = p[2];
						// Element index just chosen to be 0 initially.
						one.Past.elementIndex = one.Now.elementIndex = 0;
						// Now we successfully assign the position.
						// But, we don't know the element index of particle yet.
						// This is a bit tricky because all the particles assigned lie 
						// exactly along the edges of elements. i.e., the boundary.
						// Let's stop here for now.
					
						// Let's search the element index
						one.Past.elementIndex = one.Now.elementIndex =
							m_msh->PT->GetTheElementOfTheParticleFromNeighbor(&(one.Now));

						one.Past.identity = one.Now.identity = m_PID; 


						m_msh->PT->X.push_back(one);
						// Right, increase the particle number 
						++no;
					}
				}			

				delete [] line; delete [] particles;
			}
/*
		}
		else	// Error message
		{
			CWnd * pWnd = NULL;
			pWnd->MessageBox("Please pick only one polyline!","Info", MB_ICONINFORMATION);
		} 
*/

	}
    
    OnOK();
}

void PTValue::AssignAParticleToTheElement(Trace* one, CElem* m_ele)
{

    double unit[4];
	// Set the RWPT on
	fem->SetRWPT(1);
    fem->ConfigElement(m_ele);
    fem->ConfigNumerics(m_ele->GetElementType());
	// Turn it back off
	fem->SetRWPT(0);
   
    // Assign the index of the element
	one->Past.elementIndex = one->Now.elementIndex = m_ele->GetIndex();

	if(theApp.ElementSwitch == 1 && theApp.RFINodeSwitch != 1)
	{
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
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
			one->Past.x = one->Now.x = unit[0];
			one->Past.y = one->Now.y = unit[1];
			one->Past.z = one->Now.z = unit[2];
		}
	}
	else if(theApp.ElementSwitch == 1 && theApp.RFINodeSwitch == 1)
	{
		if(theApp.hitsRFINodeTotal > 1)
		{
			double p0[3], p1[3], a[3];
	
			p0[0] = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->X();
			p0[1] = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->Y();
			p0[2] = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->Z();
			p1[0] = m_msh->nod_vector[theApp.RFInodePickedTotal[1]]->X();
			p1[1] = m_msh->nod_vector[theApp.RFInodePickedTotal[1]]->Y();
			p1[2] = m_msh->nod_vector[theApp.RFInodePickedTotal[1]]->Z();

			double R = m_msh->PT->randomZeroToOne();
			a[0] = p1[0]-p0[0]; a[1] = p1[1]-p0[1]; a[2] = p1[2]-p0[2];
			double L = sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
			double m = L*R;
			// Normalization of this vector is necessary.
			NormalizeVector(a,3);

			// Assign the number to the particle
			one->Past.x = one->Now.x = a[0]*m + p0[0];
			one->Past.y = one->Now.y = a[1]*m + p0[1];
			one->Past.z = one->Now.z = a[2]*m + p0[2];
		}
		else
		{
			one->Past.x = one->Now.x = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->X();
			one->Past.y = one->Now.y = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->Y();
			one->Past.z = one->Now.z = m_msh->nod_vector[theApp.RFInodePickedTotal[0]]->Z();
		}
	}
}


