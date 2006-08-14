/**************************************************************************
Class: RandomWalk
Task: Random Walk - an alternative for FDM or FEM of transport equation
Programing:
07/2005 PCH Implementation
**************************************************************************/

#include "stdafx.h" //MFC
#include "rf_random_walk.h"
#include "rf_fluid_momentum.h"

// C++ STL
#include <iostream>
using namespace std;

#define PCT_FILE_EXTENSION ".pct"
#define OUTSIDEOFDOMAIN -10
//#define SWAP(x,y) {int t; t=x; x=y; y=t;};
#define SWAP(x,y) {double t; t=x; x=y; y=t;}; //WW data type is change to double

/**************************************************************************
Class: RandomWalk
Task: constructor
Programing:
07/2005 PCH Implementation
last modification:
**************************************************************************/
RandomWalk::RandomWalk(void)
{
	m_pcs = NULL;
//    m_msh = fem_msh_vector[0];  // This gotta be done better way later on need base.
    fem = NULL;

    // This is going to be reset by user input.
    // Further, used for allocating dynamic memory.
    numOfParticles = 0;
	UniformOrNormal = 1;	// Uniform random number generation
    X = NULL;

    // To produce a different pseudo-random series each time your program is run.
    srand((int)time(0));
}


// Constructor
Particle::Particle(void)
{
    // Position Vector
    x = y = z = 0.0;

    // Velocity Vector
    Vx = Vy = Vz = 0.0;

	dVxdx = dVydy = dVzdz = 0.0;

   
    // Time
    t = 0.0;
}


/**************************************************************************
Class: RandomWalk
Task: destructor
Programing:
07/2005 PCH Implementation
last modification:
**************************************************************************/
RandomWalk::~RandomWalk(void)
{
    if(X) delete [] X;
}


/**************************************************************************
Class: RandomWalk
Task: Create a random number from N(0,1) distribution
      Marsaglia Algorithm adopted...
      Refer Math 4255 Assignment
      NOTE:
      To make a use of this function, srand() should called beforehand.
      Otherwise, the random number keeps continuing the same sequence.
Programing:
08/2005 PCH Implementation
last modification:
**************************************************************************/
double RandomWalk::Marsaglia(void)
{
    int whichOne = 0;
    double u1 = 0.0, u2 = 0.0;
    double v1 = 0.0, v2 = 0.0;
    double s = 0.0;

    do
    {
        // Create two random numbers which are uniform between 0 to 1.
        u1 = (double) (1.0*rand() / (RAND_MAX+1.0));
        u2 = (double) (1.0*rand() / (RAND_MAX+1.0));

        v1 = 2. * u1 - 1.0;
        v2 = 2. * u2 - 1.0;
        s = v1*v1 + v2*v2;
    } while (s >= 1.0 || s==0.0);  // To fit log definition

    double fac = sqrt(-2.0*log(s)/s);
    
    // This will create either 0 or 1.
    whichOne = (int) (1.0*rand()/(RAND_MAX+1.0));

    if (whichOne == 0)
        return v1 * fac;
    else
        return v2 * fac;
}

double RandomWalk::randomMinusOneToOne(void)
{
    return (double)(2.0*rand()/(RAND_MAX+1.0)-1.0);
}

double RandomWalk::randomZeroToOne(void)
{
    return (double)(1.0*rand()/(RAND_MAX+1.0));
}

/**************************************************************************
Class: RandomWalk
Task: This function checks if a particle is inside of outside of the element
Programing:
08/2005 PCH Implementation
last modification:
**************************************************************************/
int RandomWalk::IsTheParticleInThisElement(Particle* A, CElem* m_ele)
{
    int InOrOut = 0; // If 1, yes it is inside. If 0, no it is outside.
    double tolerance = 1.e-6;

    // First solve the length or the area or the volume for the element
    m_ele->ComputeVolume();
    double volume = m_ele->GetVolume();
    
    // Solve the volume of the element from the particle location
    double NVolume = ComputeVolume(A, m_ele);

    // Check if two volumes are same
    if(fabs(volume - NVolume) < tolerance)
        InOrOut = 1;    // Yes it is inside of the element
    else
        InOrOut = 0;    // No, it is outside of the element
    
    return InOrOut;
}

/**************************************************************************
Class: RandomWalk
Task: This function interpolates velocity of the particle 
	  based on the inverse distance method
	  RWPT-IM This function should OK with the real plane.
Programing:
10/2005 PCH Implementation
02/2006 PCH The function is updated to solve for velocity in the element
			that has a joint or crossroads.
last modification:
**************************************************************************/
void RandomWalk::InterpolateVelocityOfTheParticleByInverseDistance(Particle* A)
{
	// Get the element that the particle belongs
	m_msh = fem_msh_vector[0]; 
	CElem* m_ele = m_msh->ele_vector[A->elementIndex];	
	// Set the pointer that leads to the nodes of element
	CNode* node = NULL;

	// Get the number of nodes
	int nnodes = m_ele->GetVertexNumber();
	// Allocate the memory accordingly
	Particle* vertex = NULL;
	vertex = new Particle [nnodes] ();
	double* d = NULL;
	d = new double [nnodes] ();
	double SumOfdInverse = 0.0;

	// Get the cooridinate of the nodes in the element 
	for(int i=0; i< nnodes; ++i)
	{
		node = m_ele->GetNode(i);
		vertex[i].x = node->X(); vertex[i].y = node->Y(); vertex[i].z = node->Z();

		// Compute the each distance
		double x= vertex[i].x-A->x; double y= vertex[i].y-A->y; double z= vertex[i].z-A->z;
		d[i] = sqrt(x*x+y*y+z*z);
		SumOfdInverse += 1.0/d[i];
	}

	// Let's get the weight of each node
	double* w = NULL;
	w = new double [nnodes] ();
	// Initialize the velocity
	A->Vx = A->Vy = A->Vz = 0.0;
	for(int i=0; i< nnodes; ++i)
	{
		w[i] = 1.0/(d[i] * SumOfdInverse);

		m_pcs = PCSGet("FLUID_MOMENTUM");
		double vx = 0.0, vy = 0.0, vz = 0.0;
		// If this node is crossroad,
		if(m_msh->nod_vector[m_ele->GetNodeIndex(i)]->crossroad == 1)
		{
			// Get the velocity contributed in this element
			CrossRoad* crossroad = NULL;
			for(int j=0; j< (int)(m_msh->fm_pcs->crossroads.size()); ++j)
				if( m_msh->fm_pcs->crossroads[j]->Index == m_msh->nod_vector[m_ele->GetNodeIndex(i)]->GetIndex() )
					crossroad = m_msh->fm_pcs->crossroads[j];

			if(crossroad)
			{
			}
			else	// Failed to find the crossroad although it is a crossroad
				abort();	

			// Find the velocity of the crossroad associated with the connected planes.
			for(int k=0; k< crossroad->numOfThePlanes; ++k)
			{
				// I am going to check the normal vector of the element and the connected plane.
				double tolerance = 1e-10;
				double E[3], P[3];
				for(int p=0; p<3; ++p)
				{
					E[p] = m_ele->getTransformTensor(6+p); 
					P[p] = crossroad->plane[k].norm[p];
				}
					
				double same = (E[0]-P[0])*(E[0]-P[0]) + (E[1]-P[1])*(E[1]-P[1]) + (E[2]-P[2])*(E[2]-P[2]);

				if(same < tolerance)
				{
					vx = crossroad->plane[k].V[0]; vy = crossroad->plane[k].V[1]; vz = crossroad->plane[k].V[2];	
				}
			}
		}
		else
		{

			vx = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);
			vy = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);
			vz = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);
		}

		A->Vx += w[i]*vx; A->Vy += w[i]*vy; A->Vz += w[i]*vz;
	}
	
	// Release the temperary memory in this function
	delete [] vertex;
	delete [] d; delete [] w;
}

/**************************************************************************
Class: RandomWalk
Task: The function solves two intersections along x or y or z axis.
	  2: The function returns two intersections
	  1: The function returns one intersection
	 -1: The function failed
	axis = 0: a line parallel to the x axis
	axis = 1: a line parallel to the y axis
	axis = 2: a line parallel to the z axis
Programing:
11/2005 PCH Implementation
02/2006 PCH Improvement for RWPT in Fracture networks.
last modification:
**************************************************************************/
int RandomWalk::SolveForTwoIntersectionsInTheElement(Particle* A, double* P1, double* P2, int axis)
{
	// Get the element that the particle belongs
	CElem* m_ele = m_msh->ele_vector[A->elementIndex];	
	// Set the pointer that leads to the nodes of element
	CNode* node = NULL;

	// Get the number of nodes
	int nnodes = m_ele->GetVertexNumber();
	// Allocate the memory accordingly
	Particle* vertex = NULL;
	vertex = new Particle [nnodes] ();
	int R = 0, L = 0;

	// Set the size of displacement
	double disp = 1e4; // This should be bigger the largest element size.
	
	// RWPT-IM
	// Get the cooridinate of the nodes in the element 
	for(int i=0; i< nnodes; ++i)
	{
		node = m_ele->GetNode(i);
		double X[3];
		X[0] = node->X(); X[1] = node->Y(); X[2] = node->Z();
		ToTheXYPlane(m_ele, X);
		vertex[i].x = X[0]; vertex[i].y = X[1]; vertex[i].z = X[2];
	}

	// Solve for the line equation
	for(int i=0; i< nnodes; ++i)
	{
		double p1[3], p2[3], p3[3], p4[3]; 
		// Need coordinate transform here.
		p1[0] = vertex[i%nnodes].x; p1[1] = vertex[i%nnodes].y; p1[2] = vertex[i%nnodes].z;
		p2[0] = vertex[(i+1)%nnodes].x; p2[1] = vertex[(i+1)%nnodes].y; p2[2] = vertex[(i+1)%nnodes].z;
		// RWPT-IM
		double X[3];
		X[0] = A->x; X[1] = A->y; X[2] = A->z;
		ToTheXYPlane(m_ele, X);
		for(int p=0; p<3; ++p) p3[p] = p4[p] = X[p];

		for(int j=0; j<2; ++j)
		{
			// See if there is an intersection in this line.
			// if a line is set to be parallel to x axis on the right
			if(axis == 0 && j==0)
				p4[0] = X[0] + disp;
			// if a line is set to be parallel to y axis on the right,
			else if(axis == 1 && j==0)
				p4[1] = X[1] + disp;
			// if a line is set to be parallel to z axis on the right,
			else if(axis == 2 && j==0)
				p4[2] = X[2] + disp;
			// if a line is set to be parallel to x axis on the left
			else if(axis == 0 && j==1)
				p4[0] = X[0] - disp;
			// if a line is set to be parallel to y axis on the left,
			else if(axis == 1 && j==1)
				p4[1] = X[1] - disp;
			// if a line is set to be parallel to z axis on the left,
			else if(axis == 2 && j==1)
				p4[2] = X[2] - disp;
			else
			{
				printf("Axis type in searching the intersection failed. Wrong axis type.\n");
				abort();
			}
		
			double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
			int status = G_intersect_line_segments( p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
													&ra, &rb, &x, &y); 
			// RWPT-IM P1 and P2 are already on the XY plane.
			if(status == 1&& j==0)
			{
				P1[0] = x; P1[1] = y; P1[2] = 0.0;
				// Transform back the coordinates.
				ToTheRealPlane(m_ele, P1);
				
				++R;
			}
			else if(status == 1&& j==1)
			{
				P2[0] = x; P2[1] = y; P2[2] = 0.0;
				// Transform back the coordinates.
				ToTheRealPlane(m_ele, P2);
				
				++L;
			}
			else;
		}		
	}

	// Free the memory for this function
	delete [] vertex;

	if(R+L == 2)
		return 2;	// The job succeeded
	else if(R+L == 1)
		return 1;
	else
		return -1; // The job failed.
}

/**************************************************************************
Class: RandomWalk
Task: The function solves three displacement by derivatives of 
	  dispersion tensor.
	  1: The function succeeded
	 -1: The function failed
Programing:
11/2005 PCH Implementation
02/2006 PCH Improved for the RWPT method in Fracture Networks.
last modification:
**************************************************************************/
int RandomWalk::SolveForDisplacementByDerivativeOfDispersion(Particle* A, double* dD)
{
	double TensorOfdD[9];

	// Solve for the derivative of velocity first
	// statusForDeivativeOfVelocity is never further used down the code.
	int statusForDeivativeOfVelocity = -10;
	statusForDeivativeOfVelocity = SolveForDerivativeOfVelocity(A);

	// Solve for the tensor of dispersion derivatives
	// Extract the dispersivities from the group that the particle belongs
	// To extract dispersivities from material properties    
	// This should be checked if the dispersivity gets correctly.
    CMediumProperties *m_mat_mp = NULL;
    double alphaL = 0.0, alphaT = 0.0;
    CElem* m_ele = m_msh->ele_vector[A->elementIndex];
    int group = m_ele->GetPatchIndex();
    m_mat_mp = mmp_vector[group];
    alphaL = m_mat_mp->mass_dispersion_longitudinal;
    alphaT = m_mat_mp->mass_dispersion_transverse;

	// RWPT - IM
	// This thing should be done on the XY plane too.
	double V[3];
	V[0] = A->Vx; V[1] = A->Vy; V[2] = A->Vz;
	ToTheXYPlane(A->elementIndex, V);
	
	double U = sqrt(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);
	
	TensorOfdD[0] = V[0]*A->dVxdx*(alphaL*(2.0/U-V[0]*V[0]/(U*U*U)) 
					- alphaT*(V[1]*V[1] + V[2]*V[2])/(U*U*U) );
	TensorOfdD[1] = (alphaL-alphaT)*(A->dVydy*V[0]/U - 
					V[0]*V[1]*V[1]/(U*U*U)*A->dVydy);
	TensorOfdD[2] = (alphaL-alphaT)*(A->dVzdz*V[0]/U - 
					V[0]*V[2]*V[2]/(U*U*U)*A->dVzdz);
	TensorOfdD[3] = (alphaL-alphaT)*(A->dVxdx*V[1]/U - 
					V[1]*V[0]*V[0]/(U*U*U)*A->dVxdx);
	TensorOfdD[4] = V[1]*A->dVydy*(alphaL*(2.0/U-V[1]*V[1]/(U*U*U)) 
					- alphaT*(V[0]*V[0] + V[2]*V[2])/(U*U*U) );
	TensorOfdD[5] = (alphaL-alphaT)*(A->dVzdz*V[1]/U - 
					V[1]*V[2]*V[2]/(U*U*U)*A->dVzdz);
	TensorOfdD[6] = (alphaL-alphaT)*(A->dVxdx*V[2]/U - 
					V[2]*V[0]*V[0]/(U*U*U)*A->dVxdx);
	TensorOfdD[7] = (alphaL-alphaT)*(A->dVydy*V[2]/U - 
					V[2]*V[1]*V[1]/(U*U*U)*A->dVydy);
	TensorOfdD[8] = V[2]*A->dVzdz*(alphaL*(2.0/U-V[2]*V[2]/(U*U*U)) 
					- alphaT*(V[0]*V[0] + V[1]*V[1])/(U*U*U) );
 	
	// Solve the three displacement by the tensor of dispersion derivative.
	dD[0] = TensorOfdD[0] + TensorOfdD[1] + TensorOfdD[2];
	dD[1] = TensorOfdD[0] + TensorOfdD[1] + TensorOfdD[2];
	dD[2] = TensorOfdD[0] + TensorOfdD[1] + TensorOfdD[2];

	return 1; 	
}

/**************************************************************************
Class: RandomWalk
Task: The function solves three main derivative of velocity. The rest of
	  the components is assumed to be zero.
	  1: The function succeeded
	 -1: The function failed
Programing:
11/2005 PCH Implementation
02/2006 PCH Improvement for fracture networks.
last modification:
**************************************************************************/
int RandomWalk::SolveForDerivativeOfVelocity(Particle* A)
{
	int status = -10;	// Set to be meaningliss in the beginning
	
	// intersections for x and y axis
	double x1[3], x2[3], y1[3], y2[3];	// I don't put the intersections for z direction for now.

	// RWPT-IM x1 and x2 are the intersection coordinates on the XY plane.
	// But the position of Particle A is on the realy plane.
	// Get the two intersecitions parallel to x axis
	status = SolveForTwoIntersectionsInTheElement(A, x1, x2, 0);
	// RWPT-IM After SolveForTwoIntersectionsInTheElement, 
	// All the coordinates are on the real plane.
	// Check if the function succeeded.
	if(status == -1)
	{
		printf("Solving two intersections parallel to x axis failed\n");
		return -1;	// Failed
	}
	// Solve for the velocity for two intersections
	Particle XR, XL;
	// RWPT-IM
	XR = XL = *A;	
	// Again, the real plane coordinates.
	XR.x = x1[0]; XR.y = x1[1]; XR.z = x1[2];
	XL.x = x2[0]; XL.y = x2[1]; XL.z = x2[2];
	
	// Interpolating velocity by the real coordinates should be no problem.
	InterpolateVelocityOfTheParticleByInverseDistance(&XR);
	InterpolateVelocityOfTheParticleByInverseDistance(&XL);
	// Solve for dVxdx
	double x = XR.x - XL.x; double y = XR.y - XL.y; double z = XR.z - XL.z; 
	double dx = sqrt(x*x + y*y + z*z);	// The distance does not make any difference.
	// RWPT-IM
	// Let me think if velocity should projected to the connected plane or treated in true 3D.
	// Yes. Velocity should be on the XY plane
	double Vx[3];
	Vx[0] = XR.Vx - XL.Vx; Vx[1] = XR.Vy - XL.Vy; Vx[2] = XR.Vz - XL.Vz;
	ToTheXYPlane(A->elementIndex, Vx);
	A->dVxdx = Vx[0] / dx; // A->dVxdx = (XR.Vx - XL.Vx) / dx; 	

	// RWPT-IM Just the same thing one more time.
	// Get the two intersecitions parallel to y axis
	status = SolveForTwoIntersectionsInTheElement(A, y1, y2, 1);
	if(status == -1)
	{
		printf("Solving two intersections parallel to y axis failed\n");
		return -1;	// Failed
	}

	// Solve for the velocity for two intersections
	Particle YR, YL;
	YR = YL = *A;
	YR.x = y1[0]; YR.y = y1[1]; YR.z = y1[2];
	YL.x = y2[0]; YL.y = y2[1]; YL.z = y2[2];
	InterpolateVelocityOfTheParticleByInverseDistance(&YR);
	InterpolateVelocityOfTheParticleByInverseDistance(&YL);
	// Solve for dVydy
	x = YR.x - YL.x; y = YR.y - YL.y; z = YR.z - YL.z; 
	double dy = sqrt(x*x + y*y + z*z);
	double Vy[3];
	Vy[0] = YR.Vx - YL.Vx; Vy[1] = YR.Vy - YL.Vy; Vy[2] = YR.Vz - YL.Vz;
	ToTheXYPlane(A->elementIndex, Vy);
	A->dVydy = Vy[1] / dy; // A->dVydy = (YR.Vy - YL.Vy) / dy;	
	

	// Just set dVzdz to be zero for now
	A->dVzdz = 0.0;
	
	// Return 1 for success
	return 1;
}

/**************************************************************************
Class: RandomWalk
Task: This function interpolates velocity of the particle
Programing:
10/2005 PCH Implementation
last modification:
**************************************************************************/
void RandomWalk::InterpolateVelocityOfTheParticle(Particle* A, CElem* m_ele)
{
    double x1buff[3];
    double x2buff[3];
    double x3buff[3];
    double x4buff[3];
    double volume = 0.0;
    double* PieceOfVolume = NULL;

    CNode* node = NULL;

    double A2buff[3];

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    // If this is not a line element, get three verteces.
    if(m_ele->GetElementType()!=1)
    {
        node = m_ele->GetNode(0);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z();
 
        node = m_ele->GetNode(1);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z();

        node = m_ele->GetNode(2);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z();
    }

    //LINES = 1 
    if (m_ele->GetElementType() == 1)
	{
        PieceOfVolume = new double[2]();
        for(int i=0; i<2; ++i)
        {
            node = m_ele->GetNode(i);
            x2buff[0] = node->X() - A2buff[0];
            x2buff[1] = node->Y() - A2buff[1];
            x2buff[2] = node->Z() - A2buff[2];
            PieceOfVolume[i] = sqrt(x2buff[0]*x2buff[0]+x2buff[1]*x2buff[1]+x2buff[2]*x2buff[2]) ;
            volume += PieceOfVolume[i];
        }
        double slope = PieceOfVolume[0] / volume;
        double V2 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(1),m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);
        double V1 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(0),m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);
        A->Vx = slope * (V2-V1) + V1;
        
        V2 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(1),m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);
        V1 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(0),m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);
        A->Vy = slope * (V2-V1) + V1;

        V2 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(1),m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);
        V1 = m_pcs->GetNodeValue(m_ele->GetNodeIndex(0),m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);
        A->Vz = slope * (V2-V1) + V1;
    }
    //RECTANGLES = 2 
	if (m_ele->GetElementType() == 2)
	{
        double tolerance = 1e-6;

        Particle* Tri1 = NULL;
        Particle* Tri2 = NULL;
                
        Tri1 = new Particle[3](); Tri2 = new Particle[3]();

        // Make two triangles from one quadlateral element
        // Tri1 : 1,2, and 4 nodes
        node = m_ele->GetNode(0);
        Tri1[0].x = node->X(); Tri1[0].y = node->Y(); Tri1[0].z = node->Z();
        node = m_ele->GetNode(1);
        Tri1[1].x = node->X(); Tri1[1].y = node->Y(); Tri1[1].z = node->Z();
        node = m_ele->GetNode(3);
        Tri1[2].x = node->X(); Tri1[2].y = node->Y(); Tri1[2].z = node->Z();
        // Tri1 : 2,3, and 4 nodes
        node = m_ele->GetNode(1);
        Tri2[0].x = node->X(); Tri2[0].y = node->Y(); Tri2[0].z = node->Z();
        node = m_ele->GetNode(2);
        Tri2[1].x = node->X(); Tri2[1].y = node->Y(); Tri2[1].z = node->Z();
        node = m_ele->GetNode(3);
        Tri2[2].x = node->X(); Tri2[2].y = node->Y(); Tri2[2].z = node->Z();

        // Compute the volume using Particle A in the triangle. 
        double VolumeOfTri1ViaParticle = ComputeVolume(A, Tri1, m_ele);
        double VolumeOfTri2ViaParticle = ComputeVolume(A, Tri2, m_ele);
        // Compute volume of each triangle
        node = m_ele->GetNode(3);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z();

        double VolumeOfTri1 = ComputeDetTri(x1buff, x2buff, x4buff);
        double VolumeOfTri2 = ComputeDetTri(x2buff, x3buff, x4buff);

        // If Particle A belongs to Tri1,
        if( fabs(VolumeOfTri1 - VolumeOfTri1ViaParticle) < tolerance)
            GetVelocityFromTriangle(A, m_ele, x1buff, x2buff, x4buff);
        // If Particle A belongs to Tri1,
        else if( fabs(VolumeOfTri2 - VolumeOfTri2ViaParticle) < tolerance)
            GetVelocityFromTriangle(A, m_ele, x2buff, x3buff, x4buff);
        else
		{
			printf("The program aborts because a particle stays none of two triangles that consist of rectangle.\n");
            abort();
		}

        delete [] Tri1;
        delete [] Tri2;
    }
    //HEXAHEDRA = 3 
	if (m_ele->GetElementType() == 3)
	{
        double tolerance = 1e-6;

        Particle* Tet1 = NULL; Particle* Tet2 = NULL; Particle* Tet3 = NULL;
        Particle* Tet4 = NULL; Particle* Tet5 = NULL; Particle* Tet6 = NULL;
                
        Tet1 = new Particle[4](); Tet2 = new Particle[4](); Tet3 = new Particle[4]();
        Tet4 = new Particle[4](); Tet5 = new Particle[4](); Tet6 = new Particle[4]();

        // Make six tetrahedras from one hexahedra element
        // Tet1 : 8,5, 4, and 6 nodes
        node = m_ele->GetNode(7);
        Tet1[0].x = node->X(); Tet1[0].y = node->Y(); Tet1[0].z = node->Z();
        node = m_ele->GetNode(4);
        Tet1[1].x = node->X(); Tet1[1].y = node->Y(); Tet1[1].z = node->Z();
        node = m_ele->GetNode(3);
        Tet1[2].x = node->X(); Tet1[2].y = node->Y(); Tet1[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet1[3].x = node->X(); Tet1[3].y = node->Y(); Tet1[3].z = node->Z();
        // Tet2 : 4, 7, 8, and 6 nodes
        node = m_ele->GetNode(3);
        Tet2[0].x = node->X(); Tet2[0].y = node->Y(); Tet2[0].z = node->Z();
        node = m_ele->GetNode(6);
        Tet2[1].x = node->X(); Tet2[1].y = node->Y(); Tet2[1].z = node->Z();
        node = m_ele->GetNode(7);
        Tet2[2].x = node->X(); Tet2[2].y = node->Y(); Tet2[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet2[3].x = node->X(); Tet2[3].y = node->Y(); Tet2[3].z = node->Z();
        // Tet3 : 4, 5, 1, and 6 nodes
        node = m_ele->GetNode(3);
        Tet3[0].x = node->X(); Tet3[0].y = node->Y(); Tet3[0].z = node->Z();
        node = m_ele->GetNode(4);
        Tet3[1].x = node->X(); Tet3[1].y = node->Y(); Tet3[1].z = node->Z();
        node = m_ele->GetNode(0);
        Tet3[2].x = node->X(); Tet3[2].y = node->Y(); Tet3[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet3[3].x = node->X(); Tet3[3].y = node->Y(); Tet3[3].z = node->Z();
        // Tet4 : 3, 7, 4, and 6 nodes
        node = m_ele->GetNode(2);
        Tet4[0].x = node->X(); Tet4[0].y = node->Y(); Tet4[0].z = node->Z();
        node = m_ele->GetNode(6);
        Tet4[1].x = node->X(); Tet4[1].y = node->Y(); Tet4[1].z = node->Z();
        node = m_ele->GetNode(3);
        Tet4[2].x = node->X(); Tet4[2].y = node->Y(); Tet4[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet4[3].x = node->X(); Tet4[3].y = node->Y(); Tet4[3].z = node->Z();
        // Tet5 : 2, 3, 4, and 6 nodes
        node = m_ele->GetNode(1);
        Tet5[0].x = node->X(); Tet5[0].y = node->Y(); Tet5[0].z = node->Z();
        node = m_ele->GetNode(2);
        Tet5[1].x = node->X(); Tet5[1].y = node->Y(); Tet5[1].z = node->Z();
        node = m_ele->GetNode(3);
        Tet5[2].x = node->X(); Tet5[2].y = node->Y(); Tet5[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet5[3].x = node->X(); Tet5[3].y = node->Y(); Tet5[3].z = node->Z();
        // Tet6 : 2, 4, 1, and 6 nodes
        node = m_ele->GetNode(1);
        Tet6[0].x = node->X(); Tet6[0].y = node->Y(); Tet6[0].z = node->Z();
        node = m_ele->GetNode(3);
        Tet6[1].x = node->X(); Tet6[1].y = node->Y(); Tet6[1].z = node->Z();
        node = m_ele->GetNode(0);
        Tet6[2].x = node->X(); Tet6[2].y = node->Y(); Tet6[2].z = node->Z();
        node = m_ele->GetNode(5);
        Tet6[3].x = node->X(); Tet6[3].y = node->Y(); Tet6[3].z = node->Z();

        // Compute the volume using Particle A in the teterahedra. 
        double VolumeOfTet1ViaParticle = ComputeVolume(A, Tet1, m_ele);
        double VolumeOfTet2ViaParticle = ComputeVolume(A, Tet2, m_ele);
        double VolumeOfTet3ViaParticle = ComputeVolume(A, Tet3, m_ele);
        double VolumeOfTet4ViaParticle = ComputeVolume(A, Tet4, m_ele);
        double VolumeOfTet5ViaParticle = ComputeVolume(A, Tet5, m_ele);
        double VolumeOfTet6ViaParticle = ComputeVolume(A, Tet6, m_ele);

        // Compute volume of each tetrahedera
        CopyParticleCoordToArray(Tet1, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet1 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Tet2, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet2 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Tet3, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet3 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Tet4, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet4 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Tet5, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet5 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Tet6, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfTet6 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);


        // If Particle A belongs to Tet1,
        if( fabs(VolumeOfTet1 - VolumeOfTet1ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet1, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet2,
        else if( fabs(VolumeOfTet2 - VolumeOfTet2ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet2, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet3,
        else if( fabs(VolumeOfTet3 - VolumeOfTet3ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet3, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet4,
        else if( fabs(VolumeOfTet4 - VolumeOfTet4ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet4, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet5,
        else if( fabs(VolumeOfTet5 - VolumeOfTet5ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet5, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet6,
        else if( fabs(VolumeOfTet6 - VolumeOfTet6ViaParticle) < tolerance)
            CopyParticleCoordToArray(Tet6, x1buff, x2buff, x3buff, x4buff);
        else
            abort();

        // Update velocity
        GetVelocityFromTetrahedra(A, m_ele, x1buff, x2buff, x3buff, x4buff);

        delete [] Tet1; delete [] Tet2; delete [] Tet3; delete [] Tet4; delete [] Tet5; delete [] Tet6;   
    }
    //TRIANGLES = 4 
	if (m_ele->GetElementType() == 4)
        GetVelocityFromTriangle(A, m_ele, x1buff, x2buff, x3buff);
    //TETRAHEDRAS = 5 
	if (m_ele->GetElementType() == 5)
	{
        node = m_ele->GetNode(3);
        x4buff[0] = node->X(); x4buff[1] = node->Y(); x4buff[2] = node->Z();

        GetVelocityFromTetrahedra(A, m_ele, x1buff, x2buff, x3buff, x4buff);
    }
    //PRISMS = 6 
	if (m_ele->GetElementType() == 6)
	{
        double tolerance = 1e-6;

        Particle* Pri1 = NULL; Particle* Pri2 = NULL; Particle* Pri3 = NULL;
                
        Pri1 = new Particle[4](); Pri2 = new Particle[4](); Pri3 = new Particle[4]();

        // Make six tetrahedras from one hexahedra element
        // Tet1 : 6, 4, 3, and 5 nodes
        node = m_ele->GetNode(5);
        Pri1[0].x = node->X(); Pri1[0].y = node->Y(); Pri1[0].z = node->Z();
        node = m_ele->GetNode(3);
        Pri1[1].x = node->X(); Pri1[1].y = node->Y(); Pri1[1].z = node->Z();
        node = m_ele->GetNode(2);
        Pri1[2].x = node->X(); Pri1[2].y = node->Y(); Pri1[2].z = node->Z();
        node = m_ele->GetNode(4);
        Pri1[3].x = node->X(); Pri1[3].y = node->Y(); Pri1[3].z = node->Z();
        // Tet1 : 3, 4, 1, and 5 nodes
        node = m_ele->GetNode(2);
        Pri2[0].x = node->X(); Pri2[0].y = node->Y(); Pri2[0].z = node->Z();
        node = m_ele->GetNode(3);
        Pri2[1].x = node->X(); Pri2[1].y = node->Y(); Pri2[1].z = node->Z();
        node = m_ele->GetNode(0);
        Pri2[2].x = node->X(); Pri2[2].y = node->Y(); Pri2[2].z = node->Z();
        node = m_ele->GetNode(4);
        Pri2[3].x = node->X(); Pri2[3].y = node->Y(); Pri2[3].z = node->Z();
        // Tet1 : 2, 3, 1, and 5 nodes
        node = m_ele->GetNode(1);
        Pri3[0].x = node->X(); Pri3[0].y = node->Y(); Pri3[0].z = node->Z();
        node = m_ele->GetNode(2);
        Pri3[1].x = node->X(); Pri3[1].y = node->Y(); Pri3[1].z = node->Z();
        node = m_ele->GetNode(0);
        Pri3[2].x = node->X(); Pri3[2].y = node->Y(); Pri3[2].z = node->Z();
        node = m_ele->GetNode(4);
        Pri3[3].x = node->X(); Pri3[3].y = node->Y(); Pri3[3].z = node->Z();

        // Compute the volume using Particle A in the teterahedra. 
        double VolumeOfPri1ViaParticle = ComputeVolume(A, Pri1, m_ele);
        double VolumeOfPri2ViaParticle = ComputeVolume(A, Pri2, m_ele);
        double VolumeOfPri3ViaParticle = ComputeVolume(A, Pri3, m_ele);
        
        // Compute volume of each tetrahedera
        CopyParticleCoordToArray(Pri1, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfPri1 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Pri2, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfPri2 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
        CopyParticleCoordToArray(Pri3, x1buff, x2buff, x3buff, x4buff);
        double VolumeOfPri3 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);

        // If Particle A belongs to Tet1,
        if( fabs(VolumeOfPri1 - VolumeOfPri1ViaParticle) < tolerance)
            CopyParticleCoordToArray(Pri1, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet2,
        else if( fabs(VolumeOfPri2 - VolumeOfPri2ViaParticle) < tolerance)
            CopyParticleCoordToArray(Pri2, x1buff, x2buff, x3buff, x4buff);
        // If Particle A belongs to Tet3,
        else if( fabs(VolumeOfPri3 - VolumeOfPri3ViaParticle) < tolerance)
            CopyParticleCoordToArray(Pri3, x1buff, x2buff, x3buff, x4buff);
        else
            abort();

        // Update velocity
        GetVelocityFromTetrahedra(A, m_ele, x1buff, x2buff, x3buff, x4buff);

        delete [] Pri1; delete [] Pri2; delete [] Pri3; 
       
    }

    // Release the memory
    delete [] PieceOfVolume;


    
}

void RandomWalk::CopyParticleCoordToArray(Particle* A, double* x1buff, 
    double* x2buff, double* x3buff, double* x4buff)
{
    x1buff[0] = A[0].x; x1buff[1] = A[0].y; x1buff[2] = A[0].z;
    x2buff[0] = A[1].x; x2buff[1] = A[1].y; x2buff[2] = A[1].z;
    x3buff[0] = A[2].x; x3buff[1] = A[2].y; x3buff[2] = A[2].z;
    x4buff[0] = A[3].x; x4buff[1] = A[3].y; x4buff[2] = A[3].z;
}

void RandomWalk::GetVelocityFromTriangle(Particle* A, CElem* m_ele, double* x1buff,
     double* x2buff,  double* x3buff)
{
    double volume = 0.0;
    double A2buff[3];
    double* PieceOfVolume = NULL;
    PieceOfVolume = new double[3]();

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    PieceOfVolume[0] = ComputeDetTri(x2buff, x3buff, A2buff) ;
    PieceOfVolume[1] = ComputeDetTri(x3buff, x1buff, A2buff) ;
    PieceOfVolume[2] = ComputeDetTri(x1buff, x2buff, A2buff) ;

    for(int i=0; i<3; ++i)
        volume += PieceOfVolume[i];    
    // Make sure v's initialized. 
    A->Vx = A->Vy = A->Vz = 0.0; 
    for(int i=0; i<3; ++i)
    {
        double vx = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);
        double vy = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);
        double vz = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);
        double weight = PieceOfVolume[i] / volume;
        A->Vx += weight * vx; A->Vy += weight * vy; A->Vz += weight * vz;
    }  

	delete [] PieceOfVolume;   
}


void RandomWalk::GetVelocityFromTetrahedra(Particle* A, CElem* m_ele, double* x1buff,
     double* x2buff,  double* x3buff, double* x4buff)
{
    double volume = 0.0;
    double A2buff[3];
    double* PieceOfVolume = NULL;
    PieceOfVolume = new double[4]();

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    PieceOfVolume[0] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;
    PieceOfVolume[1] = ComputeDetTex(A2buff, x1buff, x4buff, x3buff) ;
    PieceOfVolume[2] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
    PieceOfVolume[3] = ComputeDetTex(A2buff, x1buff, x3buff, x2buff) ;

    for(int i=0; i<4; ++i)
        volume += PieceOfVolume[i];    
    // Make sure v's initialized. 
    A->Vx = A->Vy = A->Vz = 0.0; 
    for(int i=0; i<4; ++i)
    {
        double vx = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_X")+1);
        double vy = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Y")+1);
        double vz = m_pcs->GetNodeValue(m_ele->GetNodeIndex(i),m_pcs->GetNodeValueIndex("VELOCITY1_Z")+1);
        double weight = PieceOfVolume[i] / volume;
        A->Vx += weight * vx; A->Vy += weight * vy; A->Vz += weight * vz;
    } 

	delete [] PieceOfVolume;    
}

/**************************************************************************
Class: RandomWalk
Task: This function creats the memory for particles dynamically.
Programing:
08/2005 PCH Implementation
last modification:
**************************************************************************/
void RandomWalk::CreateParticles(int HowManyParticles)
{
    // In this way, I can track the number of particle from numOfParticles.
    numOfParticles = HowManyParticles;
    // Now this X will have memory for past and now locations of all particles.
    if(X) delete X;
    X = new Trace[numOfParticles]();
}


/**************************************************************************
MSHLib-Method: 
Task:Compute the volume of the object
Programing:
09/2005 PCH Implementation
**************************************************************************/
double RandomWalk::ComputeVolume(Particle* A, CElem* m_ele)
{
    double x1buff[3];
    double x2buff[3];
    double x3buff[3];
    double x4buff[3];
    double volume = 0.0;
    double* PieceOfVolume = NULL;

    CNode* node = NULL;

    double A2buff[3];

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    // If this is not a line element, get three verteces.
    if(m_ele->GetElementType()!=1)
    {
        node = m_ele->GetNode(0);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z();
 
        node = m_ele->GetNode(1);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z();

        node = m_ele->GetNode(2);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z();
    }

    //LINES = 1 
    if (m_ele->GetElementType() == 1)
	{
        PieceOfVolume = new double[2]();
        for(int i=0; i<2; ++i)
        {
            node = m_ele->GetNode(i);
            x2buff[0] = node->X() - A2buff[0];
            x2buff[1] = node->Y() - A2buff[1];
            x2buff[2] = node->Z() - A2buff[2];
            PieceOfVolume[i] = sqrt(x2buff[0]*x2buff[0]+x2buff[1]*x2buff[1]+x2buff[2]*x2buff[2]) ;
            volume += PieceOfVolume[i];
        }
    }
    //RECTANGLES = 2 
	if (m_ele->GetElementType() == 2)
	{
        PieceOfVolume = new double[4]();
        
        node = m_ele->GetNode(3);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z();

        PieceOfVolume[0] = ComputeDetTri(x1buff, x2buff, A2buff) ;
        PieceOfVolume[1] = ComputeDetTri(x2buff, x3buff, A2buff) ;
        PieceOfVolume[2] = ComputeDetTri(x3buff, x4buff, A2buff) ;
        PieceOfVolume[3] = ComputeDetTri(x4buff, x1buff, A2buff) ;

        for(int i=0; i<4; ++i)
            volume += PieceOfVolume[i];          
    }
    //HEXAHEDRA = 3 
	if (m_ele->GetElementType() == 3)
	{
        PieceOfVolume = new double[12]();
        
        // 2,1,4,3 face
        node = m_ele->GetNode(1);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(0);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(3);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(2);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[0] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[1] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 5,6,7,8 face
        node = m_ele->GetNode(4);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(5);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(6);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(7);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[2] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[3] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 1,5,8,4 face
        node = m_ele->GetNode(0);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(4);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(7);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(3);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[4] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[5] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 8,7,3,4 face
        node = m_ele->GetNode(7);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(6);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(2);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(3);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[6] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[7] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 2,3,7,6 face
        node = m_ele->GetNode(1);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(2);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(6);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(5);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[8] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[9] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 1,2,6,5 face
        node = m_ele->GetNode(0);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(1);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(5);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(4);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[10] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[11] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        for(int i=0; i<12; ++i)
            volume += PieceOfVolume[i];    
    }
    //TRIANGLES = 4 
	if (m_ele->GetElementType() == 4)
	{
        PieceOfVolume = new double[3]();

        PieceOfVolume[0] = ComputeDetTri(x1buff, x2buff, A2buff) ;
        PieceOfVolume[1] = ComputeDetTri(x2buff, x3buff, A2buff) ;
        PieceOfVolume[2] = ComputeDetTri(x3buff, x1buff, A2buff) ;

        for(int i=0; i<3; ++i)
            volume += PieceOfVolume[i];     
    }
    //TETRAHEDRAS = 5 
	if (m_ele->GetElementType() == 5)
	{
        PieceOfVolume = new double[4]();
        
        node = m_ele->GetNode(3);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z();

        PieceOfVolume[0] = ComputeDetTex(A2buff, x1buff, x2buff, x3buff) ;
        PieceOfVolume[1] = ComputeDetTex(A2buff, x1buff, x3buff, x4buff) ;
        PieceOfVolume[2] = ComputeDetTex(A2buff, x1buff, x4buff, x2buff) ;
        PieceOfVolume[3] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        for(int i=0; i<4; ++i)
            volume += PieceOfVolume[i];   
    }
    //PRISMS = 6 
	if (m_ele->GetElementType() == 6)
	{
        PieceOfVolume = new double[8](); 

        // 2,1,3 face
        node = m_ele->GetNode(1);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(0);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(2);       
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z();
        PieceOfVolume[0] = ComputeDetTex(A2buff, x1buff, x2buff, x3buff) ;

        // 4,5,6 face
        node = m_ele->GetNode(3);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(4);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(5);       
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z();
        PieceOfVolume[1] = ComputeDetTex(A2buff, x1buff, x2buff, x3buff) ;

        // 1,4,6,3 face
        node = m_ele->GetNode(0);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(3);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(5);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(2);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[2] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[3] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 2,5,4,1 face
        node = m_ele->GetNode(1);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(4);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(3);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(0);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[4] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[5] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        // 5,2,3,6 face
        node = m_ele->GetNode(4);
        x1buff[0] = node->X();
        x1buff[1] = node->Y();
        x1buff[2] = node->Z(); 
        node = m_ele->GetNode(1);
        x2buff[0] = node->X();
        x2buff[1] = node->Y();
        x2buff[2] = node->Z(); 
        node = m_ele->GetNode(2);
        x3buff[0] = node->X();
        x3buff[1] = node->Y();
        x3buff[2] = node->Z(); 
        node = m_ele->GetNode(5);
        x4buff[0] = node->X();
        x4buff[1] = node->Y();
        x4buff[2] = node->Z(); 
        PieceOfVolume[6] = ComputeDetTex(A2buff, x1buff, x2buff, x4buff) ;
        PieceOfVolume[7] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        for(int i=0; i<8; ++i)
            volume += PieceOfVolume[i];  
    }

    // Release the memory
    delete [] PieceOfVolume;

    return volume;
}

/**************************************************************************
MSHLib-Method: 
Task:Compute the volume of the object via the particle inside of the object
Programing:
09/2005 PCH Implementation
**************************************************************************/
double RandomWalk::ComputeVolume(Particle* A, Particle* element, CElem* m_ele)
{
    double x1buff[3];
    double x2buff[3];
    double x3buff[3];
    double x4buff[3];
    double volume = 0.0;
    double* PieceOfVolume = NULL;

    double A2buff[3];

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    x1buff[0] = element[0].x; x1buff[1] = element[0].y; x1buff[2] = element[0].z; 
    x2buff[0] = element[1].x; x2buff[1] = element[1].y; x2buff[2] = element[1].z;
    x3buff[0] = element[2].x; x3buff[1] = element[2].y; x3buff[2] = element[2].z;
    
  
    //TRIANGLES = 4, RECTANGLE = 2
    int eleType = m_ele->GetElementType(); 
	if (eleType == 4 || eleType == 2)
	{
        PieceOfVolume = new double[3]();

        PieceOfVolume[0] = ComputeDetTri(x1buff, x2buff, A2buff) ;
        PieceOfVolume[1] = ComputeDetTri(x2buff, x3buff, A2buff) ;
        PieceOfVolume[2] = ComputeDetTri(x3buff, x1buff, A2buff) ;

        for(int i=0; i<3; ++i)
            volume += PieceOfVolume[i];     
    }
    //TETRAHEDRAS = 5, HEXAHEDRA = 3, PRISM = 6
	else if (eleType == 5 || eleType == 3 || eleType == 6)
	{
        PieceOfVolume = new double[4]();
        
        x4buff[0] = element[3].x; x4buff[1] = element[3].y; x4buff[2] = element[3].z;

        PieceOfVolume[0] = ComputeDetTex(A2buff, x1buff, x2buff, x3buff) ;
        PieceOfVolume[1] = ComputeDetTex(A2buff, x1buff, x3buff, x4buff) ;
        PieceOfVolume[2] = ComputeDetTex(A2buff, x1buff, x4buff, x2buff) ;
        PieceOfVolume[3] = ComputeDetTex(A2buff, x2buff, x3buff, x4buff) ;

        for(int i=0; i<4; ++i)
            volume += PieceOfVolume[i];   
    }
    else
        abort();
   
    // Release the memory
    delete [] PieceOfVolume;

    return volume;
}


/**************************************************************************
MSHLib-Method: 
Task:Compute the next positions of particles from the previous positions.
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::AdvanceParticles(double dt)
{
    // Loop over all the particles
    for(int i=0; i< numOfParticles; ++i)
    {
		Particle Y; // the displaced particle
		double V[3];
		double delta[3];
        CElem* m_ele = m_msh->ele_vector[X[i].Now.elementIndex];
		
		// Let's record the current to the past
		InterpolateVelocityOfTheParticle(&(X[i].Now), m_ele);
		V[0] = X[i].Now.Vx; V[1] = X[i].Now.Vy; V[2] = X[i].Now.Vz;
		SolveDispersionCoefficient(&(X[i].Now));
        X[i].Past = X[i].Now; 


		do
		{
			// Initialize
			X[i].Now = X[i].Past;


//			do{
				// Compute the random drift for the particle by 
				// solving the dispersion tensor at the particle location
				RandomlyDriftAway(&(X[i].Now), dt, delta, 0);
				// These delta should be transformed to locate on the plane that the element stays
				//	TransformRandomDriftToTheElementPlane(m_ele, delta);

/*
				// Now we need the dispersion tensor at the position 
				// where the particle drifted randomly by delta Of X.
				// For this I need velocity at the displaced position meaning some smart searching technique.
				// First let's assign one particle for the displaced position.
				Y.x = X[i].Now.x + delta[0]; Y.y = X[i].Now.y + delta[1]; Y.z = X[i].Now.z + delta[2];
				// Next I need to know which element has this particle.
				// This is very important since this element index is the key for extension of 
				// transport by velocity and dispersion.
				Y.elementIndex = SearchTheElementThatHasTheDisplacedParticle(&Y, m_ele);
			}while (Y.elementIndex == OUTSIDEOFDOMAIN);
            		
            // Solve the random drift for the displaced location
			if(X[i].Now.elementIndex != Y.elementIndex &&
				 -0.5 < X[i].Now.x && X[i].Now.x < 0.5 )
			{
				double dt1 = 0.0, dt2 = 0.0;
				
				// Search the boundary point
				CElem* ele1 = NULL; CElem* ele2 = NULL;
				ele1 = m_msh->ele_vector[X[i].Now.elementIndex];
				ele2 = m_msh->ele_vector[Y.elementIndex];
				CNode* commonNode = NULL;
				for(int p = 0; p < 2; ++p)
				{
					for(int q = 0; q < 2; ++q)
					{
						if(ele1->GetNodeIndex(p) == ele2->GetNodeIndex(q))
						{
							commonNode = m_msh->nod_vector[ele1->GetNodeIndex(p)];
							break;
						}
						
					}
				}
				
				double difference = X[i].Now.x - commonNode->X();
				double dx1 = sqrt(difference * difference);
				double Z = delta[0] / sqrt(6.0*X[i].Now.D[0] *dt); 
				dt1 = dx1*dx1 / (6.0*X[i].Now.D[0]*Z*Z);
				dt2 = dt - dt1;

				if(dt2 < 0.0) 
					abort();
				
				// Solve for the dispersion coefficients of Y
				SolveDispersionCoefficient(&Y);

				double D1 = X[i].Now.D[0]; double D2 = Y.D[0];
				
				double odds = sqrt(D1) / (sqrt(D1) + sqrt(D2));
				double R = randomZeroToOne();
				if(R < odds)
					RandomlyDriftAway(&(X[i].Now), dt2, delta, 0, i);	
				else
					RandomlyDriftAway(&Y, dt2, delta, 0, i);

				// Let's adjust X[i].Now location to the interface.
				X[i].Now.x = commonNode->X();
			}
	*/		
          
            // Now advance the particle by advection and dispersion
            // Advection: V[i]*dt, Dispersion: delta[i];
            X[i].Now.x += V[0]*dt + delta[0]; 
            X[i].Now.y += V[1]*dt + delta[1];
            X[i].Now.z += V[2]*dt + delta[2];
            X[i].Now.t += dt;   // Update the current time

            // Now update the current element index
            X[i].Now.elementIndex = SearchTheElementThatHasTheDisplacedParticle(&(X[i].Now), m_ele);
        }while(X[i].Now.elementIndex == OUTSIDEOFDOMAIN);
    }
}


/**************************************************************************
MSHLib-Method: 
Task:Compute the next positions of particles from the previous positions.
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::AdvanceParticlesLaBolle(double dt)
{
    // Loop over all the particles
    for(int i=0; i< numOfParticles; ++i)
    {
		Particle Y; // the displaced particle
		double V[3];
		double delta[3];
		int Dstatus = 100; int Astatus = 100;	// Set to be meaningless
        CElem* m_ele = m_msh->ele_vector[X[i].Now.elementIndex];

		// Let's record the current to the past
		printf("Velocity for X will be computed...\n");
		InterpolateVelocityOfTheParticle(&(X[i].Now), m_ele);
		V[0] = X[i].Now.Vx; V[1] = X[i].Now.Vy; V[2] = X[i].Now.Vz;
		SolveDispersionCoefficient(&(X[i].Now));
        X[i].Past = X[i].Now; 

		do
		{
			// Compute the random drift for the particle by 
			// solving the dispersion tensor at the particle location
			RandomlyDriftAway(&(X[i].Now), dt, delta, 0);
            
			// Now we need the dispersion tensor at the position 
			// where the particle drifted randomly by delta Of X.
			// For this I need velocity at the displaced position meaning some smart searching technique.
			// First let's assign one particle for the displaced position.
			Y.x = X[i].Now.x + delta[0]; Y.y = X[i].Now.y + delta[1]; Y.z = X[i].Now.z + delta[2];

			// Solve for the edge from this vector of delta
			Dstatus = SolveForDiffusionWithEdge(&(X[i].Now), &Y, delta);

			if(Dstatus == -1 || Dstatus == -2)
				printf("Dstatus = %d\n", Dstatus);
		}while (Dstatus == -1 || Dstatus == -2);

		do
		{
			// Initialize
			X[i].Now = X[i].Past;
			Y.t = dt;

			do
			{		
				RandomlyDriftAway(&Y, Y.t, delta, 0);
				
				Astatus = SolveForAdvectionWithEdge(&(X[i].Now), &Y, delta);
			
				if(Astatus == 2) Y.t = dt;

				// Update the current info
				X[i].Now = Y;

				if(Astatus == -1 || Astatus == -2)
					printf("Astatus = %d\n", Astatus);
			}while ( Y.t < dt || Astatus == -1 || Astatus == -2);
			X[i].Now.t = X[i].Past.t + dt;
        }while(Astatus == -1 || Astatus == -2);

		printf("No %d particle is done.\n", i);
    }
}

/**************************************************************************
MSHLib-Method: 
Task:The function advances the set of particles by pure advection
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::AdvanceByAdvection(double dt)
{
	double tolerance = 1e-18;
	// Loop over all the particles
    for(int i=0; i< numOfParticles; ++i)
    {
		Particle Y; // the displaced particle
		int Astatus = 100;	// Set to be meaningless
  //      CElem* m_ele = m_msh->ele_vector[X[i].Now.elementIndex];

		// Let's record the current to the past
		InterpolateVelocityOfTheParticleByInverseDistance(&(X[i].Now));
		
		// Initialize the reference and past particles
        Y=X[i].Past = X[i].Now; 

		// Initialize
		Y.t = dt;

		do
		{	
			// Let's record the current to the past
			InterpolateVelocityOfTheParticleByInverseDistance(&Y);
		
			Astatus = SolveForPureAdvectionWithEdge(&(X[i].Now), &Y);
			
			// If particle stays in the element, Y.t = dt.
			if(Y.t < tolerance) 
				Y.t = dt;

			// Update the current info
			// where the advected particle is in either the element or 
			// the neighboring element. 
			X[i].Now = Y;

			// Boundary and the function error check
			if(Astatus == -1 || Astatus == -2)
			{
				printf("Astatus = %d\n", Astatus);
				abort();
			}

		// Keep looping if the time is not spent all or
		// if the particle is outside of the domain or
		// if the function fails
		}while ( Y.t < dt );
			
		// Update the correct dt
		X[i].Now.t = X[i].Past.t + dt;

		// Keep looping 
		// if the particle is outside of the domain or
		// if the function fails
	
		printf("No %d particle is done.\n", i);
    }
}


/**************************************************************************
MSHLib-Method: 
Task:The function advances the set of particles by advection
	 and dispersion
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::AdvanceByAdvectionNDispersionSplitTime(double dt, int numOfSplit)
{
	double subdt = dt / (double)numOfSplit;

	for(int i=0; i< numOfSplit; ++i)
	{
		AdvanceByAdvectionNDispersion(subdt);
#ifdef _FEMPCHDEBUG_
	// PCH Let's monitor what's going on in the FEM
	// This messagebox is for debugging the primary variables at every time step.
	// Should combine with the picking...
	CWnd * pWnd = NULL;
	pWnd->MessageBox("Split second!!!","Debug help", MB_ICONINFORMATION);
#endif
	}

}

/**************************************************************************
MSHLib-Method: 
Task:The function advances the set of particles by advection
	 and dispersion
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::AdvanceByAdvectionNDispersion(double dt)
{
	double tolerance = 1e-18;
	// Loop over all the particles
    for(int i=0; i< numOfParticles; ++i)
    {
		Particle Y; // the displaced particle
		int Astatus = 100;	// Set to be meaningless

		if(X[i].Now.elementIndex != -10)
		{

		// Let's record the current to the past
		InterpolateVelocityOfTheParticleByInverseDistance(&(X[i].Now));
#ifdef HETERO
		SolveForDerivativeOfVelocity(&(X[i].Now));
#endif
		SolveDispersionCoefficient(&(X[i].Now));
		
		// Initialize the reference and past particles
        Y=X[i].Past = X[i].Now; 

		// Initialize
		Y.t = dt;

		do
		{	
			// Let's update the info of Particle Y.
			InterpolateVelocityOfTheParticleByInverseDistance(&Y);
#ifdef HETERO
			SolveForDerivativeOfVelocity(&Y);
#endif
			SolveDispersionCoefficient(&Y);
		
				if(Astatus == -1)
					Y.t = dt;
			Astatus = SolveForAdvectionNDispersionWithEdge(&(X[i].Now), &Y);
			
				// The result of the function is unknown error.
				if(Astatus == -2)
				{
					printf("Astatus = %d\n", Astatus);
					abort();
				}
				// Particle goes outside of the domain.
				// Thus, do it again.
				else if(Astatus == -1)
				{
					Y= X[i].Now;
				}
				// Right on track. Keep going.
				else
				{
					// If particle stays in the element, Y.t = dt.
					if(Y.t < tolerance) 
						Y.t = dt;

					// Update the current info
					// where the advected particle is in either the element or 
					// the neighboring element. 
					X[i].Now = Y;
				}			

		// Keep looping if the time is not spent all or
		// if the particle is outside of the domain or
		// if the function fails
		}while ( Y.t < dt );
			
		// Update the correct dt
		X[i].Now.t = X[i].Past.t + dt;
		}			
    }
}



/**************************************************************************
MSHLib-Method: 
Task:The function solves normalized concentration of element 
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::SetElementBasedConcentration(void)
{
/*
    double UnitConcentration = 0.0;

    // Here's definition for unit concentration
    UnitConcentration = (double)numOfParticles / (double) m_msh->ele_vector.size();
    
    for(int i=0; i< (int)m_msh->ele_vector.size(); ++i)
    {
        // Now count the number of particles in each element.
        int CountInThisElement = 0;
        for(int j=0; j< numOfParticles; ++j)
        {
            if(X[j].Now.elementIndex == i)
                ++CountInThisElement; 
        }            
      
        // Store the normalized concentration
        double NormConcentrationOfTheElement = (double)CountInThisElement / UnitConcentration;
//		double NormConcentrationOfTheElement = (double)CountInThisElement/numOfParticles;
        SetElementValue(i, GetElementValueIndex("CONCENTRATION0")+1, NormConcentrationOfTheElement);   
    }
*/

/*
	if( ((int)(X[0].Now.t*1000)%100)== 0)
	{
		char now[100];
		sprintf(now, "%f", X[0].Now.t);
		ConcPTFile(now);
	}
*/	
	
	char now[100];
	sprintf(now, "%f", X[0].Now.t);
	DATWritePCTFile(now);
}

void RandomWalk::ConcPTFile(const char *file_name)
{

	FILE *pct_file = NULL;
    char pct_file_name[MAX_ZEILE];

    CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0];  // Something must be done later on here.

    sprintf(pct_file_name,"%s.conc",file_name);
    pct_file = fopen(pct_file_name,"w+t");

	// Make a grid
	int gridDensity = 0;
	// Search Max and Min of each axis
	double MaxX = -1e6, MinX = 1e6;
	for(int i=0; i < (int)m_msh->nod_vector.size(); ++i)
	{
		CNode* thisNode = m_msh->nod_vector[i];
		if(MaxX < thisNode->X())
			MaxX = thisNode->X();
		if(MinX > thisNode->X())
			MinX = thisNode->X();
	}	

	gridDensity = (int)(MaxX - MinX);

	fprintf(pct_file, "VARIABLES = X,Y,Z,CONCENTRATION0\n");
	fprintf(pct_file, "ZONE T=\"%fs\", I=%d, F=POINT, C=BLACK\n", X[0].Now.t, gridDensity);

	for(int i=0; i < gridDensity; ++i)
	{
		int count = 0;
		double seg_start = 0.0, seg_end = 0.0;
		for(int j=0; j < numOfParticles; ++j)
		{
			seg_start = MinX + i;
			seg_end = MinX + i + 1.0;
			
			if( (X[j].Now.x >= seg_start) && (X[j].Now.x<seg_end) )
				++count;
		}
		fprintf(pct_file, "%f 0.0 0.0 %f\n", (seg_start+seg_end)/2.0, count / numOfParticles);
	}

    // Let's close it, now
    fclose(pct_file);	
}


/**************************************************************************
MSHLib-Method: 
Task:Search the index of the element in the neighber elements 
     that contains the displaced particle and return the index
Programing:
10/2005 PCH Implementation
**************************************************************************/
int RandomWalk::SearchTheElementThatHasTheDisplacedParticle(Particle* A, CElem* m_ele)
{
    int index = -10; // I intentionally set this number -10 

    // If the diplaced postion still belongs to this element
    if(IsTheParticleInThisElement(A, m_ele) == 1)
        return m_ele->GetIndex();    
    else
    {
        // Now I'm searching neighbor
        for(int i=0; i<m_ele->GetFacesNumber(); ++i)
        {
            CElem* thisNeighbor = m_ele->GetNeighbor(i);
            
            // If this neighbor is not NULL,
            if(thisNeighbor)
            {
                if(IsTheParticleInThisElement(A, thisNeighbor) == 1)
                    return thisNeighbor->GetIndex();
            }
            // This should only be the case that the line element is located
            // at the boundary after talk with WW.
            else
            {
                // I am not doing anything here. Because this is one end of the line domain.
            }
        }
        // If the code pases the following loop, it means I am not lucky in this neighbor search.
        int numberOfElements = (int)m_msh->ele_vector.size();
        for(int i=0; i< numberOfElements; ++i)
        {
            CElem* thisElement = m_msh->ele_vector[i];
            if(IsTheParticleInThisElement(A, thisElement) == 1)
                return thisElement->GetIndex();
        }
    }

    // If none of the cases above satisfies, 
    // it means the displaced particle is outside of the domain. 
    // Thus, index = -10 is never changed. 
    return index;
}


/**************************************************************************
MSHLib-Method: 
Task:Give the beat boys and free my soul. I wanna get lost in your rock &
     roll (random displace) and DRIFT AWAY. Return three component of
     random drift at the particle position. 
Programing:
10/2005 PCH Implementation
**************************************************************************/
void RandomWalk::RandomlyDriftAway(Particle* A, double dt, double* delta, int type)
{
    CElem* m_ele = m_msh->ele_vector[A->elementIndex];

    // Let's generate three random components N(0,1) and use it to compute deltaOfX
    double Z[3];

    // Here I tell the dimension for the element that contains the particle A
    int ele_dim = m_ele->GetDimension(); 
    if(ele_dim == 1)
    {
		if(UniformOrNormal == 1)
		{
			Z[0] = randomMinusOneToOne();
			delta[0] = sqrt(6.0*A->D[0]*dt) * Z[0];
			delta[1] = 0.0;
			delta[2] = 0.0;
		}
		else
		{
			Z[0] = Marsaglia(); 	
			delta[0] = sqrt(2.0*A->D[0]*dt) * Z[0];
			delta[1] = 0.0;
			delta[2] = 0.0;	
		}
        
    } else if(ele_dim == 2)
    {
		if(UniformOrNormal == 1)
		{
			Z[0] = randomMinusOneToOne(); Z[1] = randomMinusOneToOne();
			delta[0] = sqrt(6.0*A->D[0]*dt) * Z[0] + sqrt(6.0*A->D[1]*dt) * Z[1];
			delta[1] = sqrt(6.0*A->D[3]*dt) * Z[0] + sqrt(6.0*A->D[4]*dt) * Z[1];
			delta[2] = 0.0;
		}
		else
		{
			Z[0] = Marsaglia(); Z[1] = Marsaglia();
			delta[0] = sqrt(2.0*A->D[0]*dt) * Z[0] + sqrt(2.0*A->D[1]*dt) * Z[1];
			delta[1] = sqrt(2.0*A->D[3]*dt) * Z[0] + sqrt(2.0*A->D[4]*dt) * Z[1];
			delta[2] = 0.0;
		}
    } else if(ele_dim == 3)
    {
		if(UniformOrNormal == 1)
		{
			Z[0] = randomMinusOneToOne(); Z[1] = randomMinusOneToOne(); Z[2] = randomMinusOneToOne();
			delta[0] = sqrt(6.0*A->D[0]*dt) * Z[0] + sqrt(6.0*A->D[1]*dt) * Z[1] + sqrt(6.0*A->D[2]*dt) * Z[2];
			delta[1] = sqrt(6.0*A->D[3]*dt) * Z[0] + sqrt(6.0*A->D[4]*dt) * Z[1] + sqrt(6.0*A->D[5]*dt) * Z[2];
			delta[2] = sqrt(6.0*A->D[6]*dt) * Z[0] + sqrt(6.0*A->D[7]*dt) * Z[1] + sqrt(6.0*A->D[8]*dt) * Z[2];
		}
		else
		{
			Z[0] = Marsaglia(); Z[1] = Marsaglia(); Z[2] = Marsaglia();
			delta[0] = sqrt(2.0*A->D[0]*dt) * Z[0] + sqrt(2.0*A->D[1]*dt) * Z[1] + sqrt(2.0*A->D[2]*dt) * Z[2];
			delta[1] = sqrt(2.0*A->D[3]*dt) * Z[0] + sqrt(2.0*A->D[4]*dt) * Z[1] + sqrt(2.0*A->D[5]*dt) * Z[2];
			delta[2] = sqrt(2.0*A->D[6]*dt) * Z[0] + sqrt(2.0*A->D[7]*dt) * Z[1] + sqrt(2.0*A->D[8]*dt) * Z[2];
		}
    }
    else;
}

/**************************************************************************
MSHLib-Method: 
Task:The function solves random displacement by random number generation. 
	
	 
Programing:
12/2005 PCH Implementation
**************************************************************************/
int RandomWalk::RandomWalkDrift(double* Z, int dim)
{
	if(dim == 1)	// Generate the faster one.
	{
		Z[0] = 	randomMinusOneToOne();
		Z[1] = Z[2] = 0.0;

		return 1;
	}
	else if(dim == 2)	// Generate the normal distribution one
	{
		Z[0] = 	randomMinusOneToOne(); Z[1] = randomMinusOneToOne();
		Z[2] = 0.0;
		
		return 1;
	}
	else if(dim == 3)
	{
		Z[0] = randomMinusOneToOne(); Z[1] = randomMinusOneToOne(); Z[2] = randomMinusOneToOne();	
		
		return 1;
	}
	else
	{	
		printf("Something wrong in generation random drift\n");
		abort();
	}

	return -1;	// Failed.
}

/**************************************************************************
Task: SolveDispersionCoefficient(Particle* A)
Programing: This function solves velocity tensor from the velocity of
			particle.
10/2005 PCH 
02/2006 PCH	Extension to cover 2D elements in 3D
**************************************************************************/
void RandomWalk::SolveDispersionCoefficient(Particle* A)
{
	// To extract dispersivities from material properties    
    CMediumProperties *m_mat_mp = NULL;
    double alphaL = 0.0, alphaT = 0.0;
	double V[3];
    double tolerance = 1e-18;

    // Extract the dispersivities from the group that the particle belongs
	m_msh = fem_msh_vector[0]; 
    CElem* m_ele = m_msh->ele_vector[A->elementIndex];
    int group = m_ele->GetPatchIndex();
    m_mat_mp = mmp_vector[group];
    alphaL = m_mat_mp->mass_dispersion_longitudinal;
    alphaT = m_mat_mp->mass_dispersion_transverse;

    // Just solve for the magnitude of the velocity to compute the dispersion tensor
	V[0] = A->Vx; V[1] = A->Vy; V[2] = A->Vz;
	
	// RWPT-IM
	// Let's transform this velocity to be on the xy plane
	// Some nice if condition to tell the need for transform will be nice. Later....
	ToTheXYPlane(m_ele, V);
	 
    double Vmagnitude = sqrt(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);

    // Compute the dispersion tensor at the particle location
    // If the magnitude of velocity is not zero.
    if(Vmagnitude > tolerance)
    {
        A->D[0] = (alphaT*(V[1]*V[1]+ V[2]*V[2]) + alphaL*V[0]*V[0]) / Vmagnitude; // Dxx
        A->D[1] = A->D[3] = (alphaL- alphaT)*V[0]*V[1]/Vmagnitude;    // Dxy = Dyz
        A->D[2] = A->D[6] = (alphaL- alphaT)*V[0]*V[2]/Vmagnitude;    // Dxz = Dzx
        A->D[4] = (alphaT*(V[0]*V[0]+ V[2]*V[2]) + alphaL*V[1]*V[1]) / Vmagnitude; // Dyy
        A->D[5] = A->D[7] = (alphaL- alphaT)*V[1]*V[2]/Vmagnitude;    // Dyz = Dzy
        A->D[8] = (alphaT*(V[0]*V[0]+ V[1]*V[1]) + alphaL*V[2]*V[2]) / Vmagnitude; // Dzz
    }
    else
    {
		A->D[0] = alphaL; A->D[4] = alphaT;
		A->D[1] =  A->D[2] =  A->D[3] =  A->D[5] =  A->D[6] =  A->D[7] =  A->D[8] = 0.0;
    }	
}


/**************************************************************************
MSHLib-Method: 
Task:This function solves the next info of the particle by diffusion
	 1: The particle moved to the neighbor element
	 0: Particle stays in the same element.
	-1: Particle displaced outside of the domain
	-2: The function failed
Programing:
09/2005 PCH Implementation
**************************************************************************/
int RandomWalk::SolveForDiffusionWithEdge(Particle* A, Particle* B, double* delta)
{
	CElem* theElement = m_msh->ele_vector[A->elementIndex];
	int count = 0;

	// Looping over the edges of the element, A
	int nEdges = theElement->GetEdgesNumber();
	for(int i=0; i< nEdges; ++i)
	{
		// Get the edges of the element
		vec<CEdge*> theEdges(nEdges);
		theElement->GetEdges(theEdges);
		
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theEdges[i]->GetNodes(theNodes);

		double p1[3], p2[3], p3[3], p4[3];			
		p1[0] = theNodes[0]->X(); p1[1] = theNodes[0]->Y(); p1[2] = theNodes[0]->Z();
		p2[0] = theNodes[1]->X(); p2[1] = theNodes[1]->Y(); p2[2] = theNodes[1]->Z();
		p3[0] = A->x; p3[1] = A->y; p3[2] = A->z;
		// Intentionally make the segment of p3 and p4 big.
		p4[0] = p3[0] + delta[0]; p4[1] = p3[1] + delta[1]; p4[2] = p3[2] + delta[2];
		
		double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
		int status = G_intersect_line_segments (p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
							&ra, &rb, &x, &y); 
		
		// If intersection is a sinle point
		if(status == 1)
		{
			// Find the neighbor element and update
			// Find the common edge
			// The common edge is theEdges[i]
			
			// Now I'm searching neighbor
			int countBoundary = 0;	// Set to be outside of boundary
			for(int j=0; j<theElement->GetFacesNumber(); ++j)
			{
				CElem* thisNeighbor = theElement->GetNeighbor(j);

				// Get the edges of each neighbor
				int neighborEdgesNumber = thisNeighbor->GetEdgesNumber();
				vec<CEdge*> theNeighborEdges(neighborEdgesNumber);
				thisNeighbor->GetEdges(theNeighborEdges);	
				for(int k=0; k< neighborEdgesNumber; ++k)
				{
					if(theEdges[i] == theNeighborEdges[k])	
					{
						B->elementIndex = thisNeighbor->GetIndex(); 
				//		B->x += delta[0]; B->y += delta[1]; B->z += delta[2];
						countBoundary = 1;

						return 1;	// The element index switched to the neighbor element
					}
				}	
			}
			// Check if there is no neighboring element
			// meaning outside of the boundary
			if(countBoundary == 0)
			{
				B->elementIndex = -10;	// Outside of the domain	
				return -1;	// Particle displaced outside of the domain
			}			
		}
		// no intersection but the extension has one.
		else if(status == 0)
			++count;
		else
		{
			printf("The program aborts because status of intersection search is not 1 or 0.\n");
			abort();
		}
	}

	// If none of the edges intersect by this delta vector
	if(count == nEdges)
	{
		// Just record the element index of the original particle position
		B->elementIndex = A->elementIndex;
//		B->x += delta[0]; B->y += delta[1]; B->z += delta[2];

		return 0; // Particle stays in the same element.
	}

	return -2;	// The function failed
}

/**************************************************************************
MSHLib-Method: 
Task:This function solves the next info of the particle by advection 
	 and dispersion
	 2: The particle is at the intersection
	 1: The particle moved to the neighbor element
	 0: Particle stays in the same element. 
	-1: Particle displaced outside of the domain
	-2: The function failed
Programing:
09/2005 PCH Implementation
**************************************************************************/
int RandomWalk::SolveForAdvectionWithEdge(Particle* A, Particle* B, double* delta)
{
	CElem* theElement = m_msh->ele_vector[B->elementIndex];
	int count = 0; 

	int nEdges = theElement->GetEdgesNumber();
	for(int i=0; i< nEdges; ++i)
	{
		// Get the edges of the element
		vec<CEdge*> theEdges(nEdges);
		theElement->GetEdges(theEdges);
		
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theEdges[i]->GetNodes(theNodes);

		double p1[3], p2[3], p3[3], p4[3];			
		// Two points in the edge
		p1[0] = theNodes[0]->X(); p1[1] = theNodes[0]->Y(); p1[2] = theNodes[0]->Z();
		p2[0] = theNodes[1]->X(); p2[1] = theNodes[1]->Y(); p2[2] = theNodes[1]->Z();
		// The starting point displaced by diffusion as in LaBolle
		p3[0] = B->x; p3[1] = B->y; p3[2] = B->z;
		// Intentionally make the segment of p3 and p4 big.
		printf("Velocity for Y will be computed...\n");
		printf("The element index of Y is %d.\n", B->elementIndex);
		InterpolateVelocityOfTheParticle(B, theElement);
		
		SolveDispersionCoefficient(B);
		p4[0] = p3[0] + B->Vx * B->t + delta[0]; 
		p4[1] = p3[1] + B->Vy * B->t + delta[1]; 
		p4[2] = p3[2] + B->Vz * B->t + delta[2];

		double dtt = B->t;
		
		double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
		int status = G_intersect_line_segments( p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
							&ra, &rb, &x, &y); 

		if( (B->x != x) && (B->y != y) )
		{
		
			// If intersection is a sinle point
			if(status == 1)
			{
				// Find the neighbor element and update
				// Find the common edge
				// The common edge is theEdges[i]
			
				// Now I'm searching neighbor
				int countBoundary = 0;	// Set to be outside of boundary
				for(int j=0; j<theElement->GetFacesNumber(); ++j)
				{
					CElem* thisNeighbor = theElement->GetNeighbor(j);

					// Get the edges of each neighbor
					int neighborEdgesNumber = thisNeighbor->GetEdgesNumber();
					vec<CEdge*> theNeighborEdges(neighborEdgesNumber);
					thisNeighbor->GetEdges(theNeighborEdges);
		
					for(int k=0; k< neighborEdgesNumber; ++k)
					{
						if(theEdges[i] == theNeighborEdges[k])	
						{
							B->elementIndex = thisNeighbor->GetIndex(); 
							countBoundary = 1;
						
							// Compute the time left over.
							double dt1 = (dtt*ra)/(ra+1.0);
							double dt2 = dtt - dt1;

							if(dt2 < 0.0)
							{ 
								printf("The program aborts because dt2 < 0.0\n");
								abort();
							}
							// Record the time left and update the position to be the intersection
							B->t = dt2; 
							B->x = x; B->y = y; B->z = 0.0; 

							return 1;	// The element index switched to the neighbor element
						}
					}	
				}		
				// Check if there is no neighboring element
				// meaning outside of the boundary
				if(countBoundary == 0)
				{
					B->elementIndex = -10;	// Outside of the domain
					return -1;	// Particle displaced outside of the domain	
				}		
			}
			// It couldn't reach to the edge
			else if(status == 0)
				++count;
			else
			{
				printf("The program aborts because status of intersection search is not 1 or 0.\n");
				abort();
			}
		}
		else
		{
			B->x += B->Vx * B->t + delta[0];
			B->y += B->Vy * B->t + delta[1]; 
			B->y += B->Vz * B->t + delta[2];
			
			return 2;	//The particle is at the intersection
		}
	}

	// If none of the edges intersect by this delta vector
	if(count == nEdges)
	{
		B->elementIndex = A->elementIndex;
		B->x += B->Vx * B->t + delta[0];
		B->y += B->Vy * B->t + delta[1]; 
		B->y += B->Vz * B->t + delta[2];
		return 0; // Particle stays in the same element.
	}
	
	return -2;	// The function failed
}


/**************************************************************************
MSHLib-Method: 
Task:This function solves the next info of the particle by advection only
	 1: The particle moved to the neighbor element
	 0: Particle stays in the same element. 
	-1: Particle displaced outside of the domain
	-2: The function failed
Programing:
09/2005 PCH Implementation
**************************************************************************/
int RandomWalk::SolveForAdvectionNDispersionWithEdge(Particle* A, Particle* B)
{
	m_msh = fem_msh_vector[0]; 
	CElem* theElement = m_msh->ele_vector[B->elementIndex];
	
	// Getting the number of the edges in the element that Particle P belongs
	int nEdges = theElement->GetEdgesNumber();
	int countNoIntersection = 0;

	// The estimated position advected for the given B->t
	double dD[3]; 
	double Z[3];
	int ele_dim = theElement->GetDimension(); 

	for(int i=0; i < 3; ++i)
		dD[i] = 0.0;

	// Loop over the edges
	for(int i=0; i< nEdges; ++i)
	{
		// Get the edges of the element
		vec<CEdge*> theEdges(nEdges);
		theElement->GetEdges(theEdges);
		
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theEdges[i]->GetNodes(theNodes);

		double p1[3], p2[3], p3[3], p4[3];			
		// RWPT - IM 		
		// Two points in the edge
		double X1[3], X2[3];
		X1[0] = theNodes[0]->X(); X1[1] = theNodes[0]->Y(); X1[2] = theNodes[0]->Z();
		X2[0] = theNodes[1]->X(); X2[1] = theNodes[1]->Y(); X2[2] = theNodes[1]->Z();
		ToTheXYPlane(theElement, X1); ToTheXYPlane(theElement, X2);
		for(int j=0; j<3; ++j)
		{
			p1[j] = X1[j];	p2[j] = X2[j];
		}
		// The starting point displaced by pure advection
		p3[0] = B->x; p3[1] = B->y; p3[2] = B->z;
		ToTheXYPlane(theElement, p3);

		int dDStatus = 1;
#ifdef HETERO		
		// This currently only return TRUE (1)
		dDStatus = SolveForDisplacementByDerivativeOfDispersion(A, dD);
#endif

		// Let's get the local vector for particle velocity
		double V[3];

		// Create random drift according to the element dimension
		RandomWalkDrift(Z, ele_dim);
		if(dDStatus == 1)
		{
			if(ele_dim == 2)
			{
				// RWPT - IM 
				// This should be done carefully. Velocity should be transformed to be on the XY plane.
				// dD[] should be fine because it is handled in the SolveForDisplacementByDerivativeOfDispersion function.
				// Z[] should also be fine. Just randome nubmers.
				// D[] Yes, this should be fine too. It is handled in the SolveDispersionCoefficient function.
				// OK. Just velocity left.
				V[0] = B->Vx;	V[1] = B->Vy;	V[2] = B->Vz;	// In fact, V[2] gotta be zero.
				ToTheXYPlane(B->elementIndex, V);
				p4[0] = p3[0] + V[0] * B->t + dD[0]*B->t + sqrt(6.0*B->D[0]*B->t) * Z[0] + sqrt(6.0*fabs(B->D[1])*B->t) * Z[1]; 
				p4[1] = p3[1] + V[1] * B->t + dD[1]*B->t + sqrt(6.0*fabs(B->D[3])*B->t) * Z[0] + sqrt(6.0*B->D[4]*B->t) * Z[1]; 
				// Fix for translation
				p4[2] = theElement->GetAngle(2);
			}
			else
			{
				printf("Other dimensions are not implemented yet.\n");
				abort();
			}
		}
		else
		{
			// This should never be the case by now. Later on, maybe.
			printf("SolveForDisplacementByDerivativeOfDispersion failed\n");
			abort();
		}

		// Initialize the values for getting the intersection
		double dtt = B->t;
		double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
		int status = G_intersect_line_segments( p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
							&ra, &rb, &x, &y); 

		// If intersection is a sinle point
		if(status == 1)
		{
			// Find the neighbor element and update
			// Find the common edge
			// The common edge is theEdges[i]
			
			// Now I'm searching neighbor
			for(int j=0; j<theElement->GetFacesNumber(); ++j)
			{
				// Compute the time left over.
				double I[3];
				// Fix for translation
				I[0] = x; I[1] = y; I[2] = theElement->GetAngle(2);

				double d1 = SolveDistanceBetweenTwoPoints(p3, I);
				double d = SolveDistanceBetweenTwoPoints(p3, p4);
				double dt1 = dtt*d1/d;
				double dt2 = dtt - dt1;

						// dt2 should be positive
						if(dt2 < 0.0)
						{ 
							printf("The program aborts because dt2 < 0.0\n");
							abort();
						}
				else;	// keep going.

				// Update the record.
				B->t = dt2; 
				// Adjust the position for the obtained dt1.
				double Dxx = sqrt(6.0*B->D[0]*dt1) * Z[0]; double Dxy = sqrt(6.0*fabs(B->D[1])*dt1) * Z[1];
				double Dyx = sqrt(6.0*fabs(B->D[3])*dt1) * Z[0]; double Dyy = sqrt(6.0*B->D[4]*dt1) * Z[1];
				double dsp[3];
				dsp[0] = dD[0]*dt1 + Dxx + Dxy; 
				dsp[1] = dD[1]*dt1 + Dyx + Dyy;
				// Fix for translation
				dsp[2] = theElement->GetAngle(2);

				double IC[3];
				// Fix for translation
				IC[0] = x + dsp[0]; IC[1] = y + dsp[1]; IC[2] = theElement->GetAngle(2);
				// Let's convert these XY plance coordinates to the real plane coordinates.
				ToTheRealPlane(B->elementIndex, IC); 							
				B->x = IC[0]; B->y = IC[1]; B->z = IC[2]; 
				// The difference is that I need to update the element index.
				B->elementIndex = GetTheElementOfTheParticleFromNeighbor(B);

				if(B->elementIndex == -10)
				{
					printf("There's single intersection.\n But, it's going outside of the domain.");
					return -1;
				}

				return 1;	// The element index switched to the neighbor element
			}

		}
		// It couldn't reach to the edge
		else if(status == 0)
			++countNoIntersection;
		// If two segments are parallel
		else if(status == -1)
		{
			++countNoIntersection;
			// keep going.
		}
		// If two segments are colinear
		else if(status == 2)
		{
			printf("The program aborts because two segments are colinear.\n");
			abort();
		}
		else
		{
			printf("The program aborts because status of intersection search is not 1 or 0 or -1.\n");
			abort();
		}
	}
	// Check if the time left advances the particle within this element
	if(countNoIntersection == nEdges)
	{
		if(ele_dim == 2)
		{
			double V[3];
			V[0] = B->Vx;	V[1] = B->Vy;	V[2] = B->Vz;	// In fact, V[2] gotta be zero.
			ToTheXYPlane(B->elementIndex, V);	// V XY planed.

			double Dxx = sqrt(6.0*B->D[0]*B->t) * Z[0]; double Dxy = sqrt(6.0*fabs(B->D[1])*B->t) * Z[1];
			double Dyx = sqrt(6.0*fabs(B->D[3])*B->t) * Z[0]; double Dyy = sqrt(6.0*B->D[4]*B->t) * Z[1];

			double dsp[3];
			dsp[0] = V[0]*B->t + dD[0]*B->t + Dxx + Dxy; 
			dsp[1] = V[1]*B->t + dD[1]*B->t + Dyx + Dyy;
			// Fix for translation
			//	dsp[2] = 0.0;	// This gotta be zero all the times in fracture networks.
			dsp[2] = theElement->GetAngle(2);
								
			// Assigning the next postion of the particle. The index of element in this if condition
			// should be one of the connected planes randomly chosen.
			// Now just solve the real plane coordinates for the particle at the next position.
			double DspInNextEle[3];
			for(int p=0; p<3; ++p) DspInNextEle[p] = dsp[p]; 
			
			double P[3];
			P[0] = B->x; P[1] = B->y; P[2] = B->z;
			ToTheXYPlane(B->elementIndex, P);
			P[0] += DspInNextEle[0]; P[1] += DspInNextEle[1]; P[2] = theElement->GetAngle(2);
			ToTheRealPlane(B->elementIndex, P);
			B->x = P[0]; B->y = P[1]; B->z = P[2];
			
			// Now, for the index of the element that this moved particle stays, 
			// we have no idea. Thus, need to search.
			// But, this is very costly process. I should think about this
			// for speeding up later on.
			B->elementIndex = GetTheElementOfTheParticleFromNeighbor(B);
		}
		else
		{
			printf("Other dimensions are not implemented yet.\n");
			abort();
		}

		B->t = 0.0;

		return 0; // Particle stays in the same element.	
	}
	else
		return -2;	// The function failed
}



/**************************************************************************
MSHLib-Method: 
Task:This function returns the index of the element that contains 
	 the particle from neighboring elements only.
Programing:
12/2005 PCH Implementation
**************************************************************************/
int RandomWalk::GetTheElementOfTheParticleFromNeighbor(Particle* A)
{
	int index = -10;

	m_msh = fem_msh_vector[0]; 
	CElem* theElement = m_msh->ele_vector[A->elementIndex];
	// Let's check this element first.
	index = IsTheParticleInThisElement(A);
	if(index != -1)	
		return index;
	
	// First meighbor's search around the main element
	for(int i=0; i<theElement->GetFacesNumber(); ++i)
	{
		CElem* thisNeighbor = theElement->GetNeighbor(i);
				
		if(thisNeighbor) 
		{
			// If the particle belongs to this element
			A->elementIndex = thisNeighbor->GetIndex();
			index = IsTheParticleInThisElement(A);
			if(index != -1)	
				return index;
		}
		else
		{
			printf("Searching the first loop of neighbor failed.\n");
			abort();
		}
		
		// Second, search the neighbor's neighbor
		for(int j=0; j<thisNeighbor->GetFacesNumber(); ++j)
		{
			CElem* theNeighborsNeighbor = thisNeighbor->GetNeighbor(j);	

			if(theNeighborsNeighbor)
			{
				// If the particle belongs to this element
				A->elementIndex = theNeighborsNeighbor->GetIndex();
				index = IsTheParticleInThisElement(A);
				if(index != -1)	
					return index;	
			}
			else
			{
				printf("Searching the second loop of neighbor failed.\n");
				abort();
			}

			// Third, search the neighbor's neighbor's neighbor
			for(int k=0; k< theNeighborsNeighbor->GetFacesNumber(); ++k)
			{
				CElem* theNeighborsNeighborsNeighbor = theNeighborsNeighbor->GetNeighbor(k);

				if(theNeighborsNeighborsNeighbor)
				{
					// If the particle belongs to this element
					A->elementIndex = theNeighborsNeighborsNeighbor->GetIndex();
					index = IsTheParticleInThisElement(A);
					if(index != -1)	
						return index;
				}
				else
				{
					printf("Searching the third loop of neighbor failed.\n");
					abort();
				}
			}
		}	
	}




	// If the code pases the following loop, it means I am not lucky in this neighbor search.
    int numberOfElements = (int)m_msh->ele_vector.size();
    for(int i=0; i< numberOfElements; ++i)
    {
        CElem* thisElement = m_msh->ele_vector[i];
       
		if(thisElement) 
		{
			// If the particle belongs to this element
			A->elementIndex = thisElement->GetIndex();
			index = IsTheParticleInThisElement(A);
			if(index != -1)	
				return index;
		}
		else
		{
			printf("The exhaustive search failed.\n");
			abort();
		}
    }
	
	// The search failed
	if(index == -1)	index = -10;
	printf("Searching the index from the neighbor failed\n");
	return index;
	
}

/**************************************************************************
MSHLib-Method: 
Task:Search the index of the element in the neighber elements 
     that contains the displaced particle and return the index
Programing:
10/2005 PCH Implementation
**************************************************************************/
int RandomWalk::GetTheElementOfTheParticleFromNeighborAggressive(Particle* A)
{
    int index = -10; // I intentionally set this number -10 

	CElem* theElement = m_msh->ele_vector[A->elementIndex];
	
	// First meighbor's search around the main element
	for(int i=0; i<theElement->GetFacesNumber(); ++i)
	{
		CElem* thisNeighbor = theElement->GetNeighbor(i);
				
		if(thisNeighbor) 
		{
			// If the particle belongs to this element
			A->elementIndex = thisNeighbor->GetIndex();
			index = IsTheParticleInThisElement(A);
			if(index != -1)	
				return index;
		}
		else
		{
			printf("Searching the first loop of neighbor failed.\n");
			abort();
		}
	}
        
	// If the code pases the following loop, it means I am not lucky in this neighbor search.
    int numberOfElements = (int)m_msh->ele_vector.size();
    for(int i=0; i< numberOfElements; ++i)
    {
        CElem* thisElement = m_msh->ele_vector[i];
       
		if(thisElement) 
		{
			// If the particle belongs to this element
			A->elementIndex = thisElement->GetIndex();
			index = IsTheParticleInThisElement(A);
			if(index != -1)	
				return index;
		}
		else
		{
			printf("Searching exhastively failed.\n");
			abort();
		}
    }

    // If none of the cases above satisfies, 
    // it means the displaced particle is outside of the domain. 
    // Thus, index = -10 is never changed. 
    return index;
}

/**************************************************************************
MSHLib-Method: 
Task:This function returns the index of the element that contains 
	 the particle if the particle exists in the element.
	 Or return -1 if the particle is not in the element.
Programing:
12/2005 PCH Implementation
02/2006 PCH Improved for the RWPT method in Fracture Networks.
02/2006 PCH The ray method implemented based on the proven theory.
**************************************************************************/
int RandomWalk::IsTheParticleInThisElement(Particle* A)
{
	m_msh = fem_msh_vector[0]; 
	CElem* theElement = m_msh->ele_vector[A->elementIndex];
	
	// Getting the number of the edges in the element that Particle P belongs
	int nEdges = theElement->GetEdgesNumber();
	int countOfInterception = 0;
	int parallel = 0;
	// Loop over the edges
	for(int i=0; i< nEdges; ++i)
	{
		// Get the edges of the element
		vec<CEdge*> theEdges(nEdges);
		theElement->GetEdges(theEdges);
		
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theEdges[i]->GetNodes(theNodes);

		double p1[3], p2[3], p3[3], p4[3];	
		// RWPT - IM 		
		// Two points in the edge
		double X1[3], X2[3];
		X1[0] = theNodes[0]->X(); X1[1] = theNodes[0]->Y(); X1[2] = theNodes[0]->Z();
		X2[0] = theNodes[1]->X(); X2[1] = theNodes[1]->Y(); X2[2] = theNodes[1]->Z();
		ToTheXYPlane(theElement, X1); ToTheXYPlane(theElement, X2);
		for(int j=0; j<3; ++j)
		{
			p1[j] = X1[j];	p2[j] = X2[j];
		}

		// The starting point which is the particle position
		p3[0] = A->x; p3[1] = A->y; p3[2] = A->z;
		// RWPT - IM 
		ToTheXYPlane(theElement, p3);
		// Make p4 very long in x direction in the XY plane
		double big = 1e3;
		p4[0] = p3[0] + big; p4[1] = p3[1]; p4[2] = p3[2];
		
		double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
		int status = G_intersect_line_segments( p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
							&ra, &rb, &x, &y); 

		if(status == 0)		// Not intersect but extension intersects
			;
		else if(status == -1) // Parallel
			parallel = 1;
		else if(status == 1)
			++countOfInterception; // single intersection
		else if(status == 2)	// Overlap (this should be impossible)
		{
			printf("Overlapping in IsThisInElement is impossible.\n");
			abort();
		}
		else
		{
			printf("Not making any sense.\n");
			abort();
		}
		
	}
	// Check if this particle is inside of the element
	// If the number of interceptions is odd,
	// then, it is inside of this element.
	if(countOfInterception%2 == 1)
		return A->elementIndex;
	// if the number is even,
	// then, it is outside
	else
		return -1; // This element does not have the particle.
}

/**************************************************************************
MSHLib-Method: 
Task:This function solves the distance between two points
Programing:
09/2005 PCH Implementation
**************************************************************************/
double RandomWalk::SolveDistanceBetweenTwoPoints(double* p1, double* p2)
{
	double x = p2[0]-p1[0]; double y = p2[1]-p1[1]; double z = p2[2]-p1[2];

	return sqrt(x*x + y*y + z*z);
}


/**************************************************************************
MSHLib-Method: 
Task:This function solves the next info of the particle by advection only
	 1: The particle moved to the neighbor element
	 0: Particle stays in the same element. 
	-1: Particle displaced outside of the domain
	-2: The function failed
Programing:
09/2005 PCH Implementation
**************************************************************************/
int RandomWalk::SolveForPureAdvectionWithEdge(Particle* A, Particle* B)
{
	CElem* theElement = m_msh->ele_vector[B->elementIndex];
	
	// Getting the number of the edges in the element that Particle P belongs
	int nEdges = theElement->GetEdgesNumber();
	int countNoIntersection = 0;
	// Loop over the edges
	for(int i=0; i< nEdges; ++i)
	{
		// Get the edges of the element
		vec<CEdge*> theEdges(nEdges);
		theElement->GetEdges(theEdges);
		
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theEdges[i]->GetNodes(theNodes);

		double p1[3], p2[3], p3[3], p4[3];			
		// Two points in the edge
		p1[0] = theNodes[0]->X(); p1[1] = theNodes[0]->Y(); p1[2] = theNodes[0]->Z();
		p2[0] = theNodes[1]->X(); p2[1] = theNodes[1]->Y(); p2[2] = theNodes[1]->Z();
		// The starting point displaced by pure advection
		p3[0] = B->x; p3[1] = B->y; p3[2] = B->z;
		
		// The estimated position advected for the given B->t
		p4[0] = p3[0] + B->Vx * B->t; 
		p4[1] = p3[1] + B->Vy * B->t; 
		p4[2] = p3[2] + B->Vz * B->t;

		// Initialize the values for getting the intersection
		double dtt = B->t;
		double x = 0.0, y = 0.0, ra = 0.0, rb = 0.0;
		
		int status = G_intersect_line_segments( p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1],
							&ra, &rb, &x, &y); 

		// If intersection is a sinle point
		if(status == 1)
		{
			// Find the neighbor element and update
			// Find the common edge
			// The common edge is theEdges[i]
			
			// Now I'm searching neighbor
			int countOutside = 0;
			for(int j=0; j<theElement->GetFacesNumber(); ++j)
			{
				CElem* thisNeighbor = theElement->GetNeighbor(j);
				
				if(thisNeighbor) ++countOutside;

				// Get the edges of each neighbor
				int neighborEdgesNumber = thisNeighbor->GetEdgesNumber();
				vec<CEdge*> theNeighborEdges(neighborEdgesNumber);
				thisNeighbor->GetEdges(theNeighborEdges);
		
				for(int k=0; k< neighborEdgesNumber; ++k)
				{
					// If the edge is shared by two elements
					if(theEdges[i] == theNeighborEdges[k])	
					{
						// Compute the time left over.
						double xx = p3[0] - x; double yy = p3[1] - y; double zz = p3[2] - 0.0;
						double distance = sqrt(xx*xx + yy*yy + zz*zz);
						double VV = sqrt(B->Vx*B->Vx + B->Vy*B->Vy + B->Vz*B->Vz);
						double dt1 = distance/VV;
						double dt2 = dtt - dt1;

						// dt2 should be positive
						if(dt2 < 0.0)
						{ 
							printf("The program aborts because dt2 < 0.0\n");
							abort();
						}
						else;	// keep going.

						// Check if the edge is the right edge by comparing the direction 
						// of two vectors 
						double OVec[3];	double EVec[3];
						OVec[0] = p4[0] - p3[0]; OVec[1] = p4[1] - p3[1]; OVec[2] = p4[2] - p3[2];
						EVec[0] = x - p3[0]; EVec[1] = y - p3[1]; EVec[2] = 0.0;
						double dotProduct = OVec[0]*EVec[0] + OVec[1]*EVec[1] + OVec[2]*EVec[2]; 
						double tolerance = 1e-18;

						// If the direction of two vectors is same,
						if(dotProduct > 0.0)
						{
							// Update the record.
							B->elementIndex = thisNeighbor->GetIndex(); 
							B->t = dt2; 
							B->x = x; B->y = y; B->z = 0.0; 
					
							return 1;	// The element index switched to the neighbor element
						}
						else if(fabs(dotProduct) < tolerance) // The crossing segment is the point of the intersection
						{	
							return 0;	// The element index switched to the neighbor element	
						}
						else if(dotProduct < 0.0) ; // The direction of two vectors is opposite.
					}
					else;	// No intersection.
						
				}	
		
			}

			// Check if the particle is outside of the domain
			if(countOutside == nEdges)
			{
				// Particle steps outside of the problem domain	
				B->elementIndex = -10;	// Outside of the domain
						
				return -1;	// Particle displaced outside of the domain	
			}
			else
			{
				// Something wrong.
				printf("There must be something wrong in this algorithm\n");
				abort();
			}
		}
		// It couldn't reach to the edge
		else if(status == 0)
			++countNoIntersection;
		// If two segments are parallel
		else if(status == -1)
		{
			++countNoIntersection;
			// keep going.
		}
		// If two segments are colinear
		else if(status == 2)
		{
			printf("The program aborts because two segments are colinear.\n");
			abort();
		}
		else
		{
			printf("The program aborts because status of intersection search is not 1 or 0 or -1.\n");
			abort();
		}
	}

	// Check if the time left advances the particle within this element
	if(countNoIntersection == nEdges)
	{
		B->elementIndex = A->elementIndex;
		B->x += B->Vx * B->t;
		B->y += B->Vy * B->t; 
		B->y += B->Vz * B->t;
		B->t = 0.0;

		return 0; // Particle stays in the same element.
	}
	else
		return -2;	// The function failed
}

/**************************************************************
* find interesection between two lines defined by points on the lines
* line segment A is (ax1,ay1) to (ax2,ay2)
* line segment B is (bx1,by1) to (bx2,by2)
* returns
*   -1 segment A and B do not intersect (parallel without overlap)
*    0 segment A and B do not intersect but extensions do intersect
*    1 intersection is a single point
*    2 intersection is a line segment (colinear with overlap)
* x,y intersection point
* ra - ratio that the intersection divides A 
* rb - ratio that the intersection divides B
*
*                              B2
*                              /
*                             /
*   r=p/(p+q) : A1---p-------*--q------A2
*                           /
*                          /
*                         B1
*
**************************************************************/
  
/**************************************************************
*
* A point P which lies on line defined by points A1=(x1,y1) and A2=(x2,y2)
* is given by the equation r * (x2,y2) + (1-r) * (x1,y1).
* if r is between 0 and 1, p lies between A1 and A2.
* 
* Suppose points on line (A1, A2) has equation 
*     (x,y) = ra * (ax2,ay2) + (1-ra) * (ax1,ay1)
* or for x and y separately
*     x = ra * ax2 - ra * ax1 + ax1
*     y = ra * ay2 - ra * ay1 + ay1
* and the points on line (B1, B2) are represented by
*     (x,y) = rb * (bx2,by2) + (1-rb) * (bx1,by1)
* or for x and y separately
*     x = rb * bx2 - rb * bx1 + bx1
*     y = rb * by2 - rb * by1 + by1
* 
* when the lines intersect, the point (x,y) has to
* satisfy a system of 2 equations:
*     ra * ax2 - ra * ax1 + ax1 = rb * bx2 - rb * bx1 + bx1
*     ra * ay2 - ra * ay1 + ay1 = rb * by2 - rb * by1 + by1
* 
* or
* 
*     (ax2 - ax1) * ra - (bx2 - bx1) * rb = bx1 - ax1
*     (ay2 - ay1) * ra - (by2 - by1) * rb = by1 - ay1
* 
* by Cramer's method, one can solve this by computing 3
* determinants of matrices:
* 
*    M  = (ax2-ax1)  (bx1-bx2)
*         (ay2-ay1)  (by1-by2)
* 
*    M1 = (bx1-ax1)  (bx1-bx2)
*         (by1-ay1)  (by1-by2)
* 
*    M2 = (ax2-ax1)  (bx1-ax1)
*         (ay2-ay1)  (by1-ay1)
* 
* Which are exactly the determinants D, D2, D1 below:
* 
*   D  ((ax2-ax1)*(by1-by2) - (ay2-ay1)*(bx1-bx2))
* 
*   D1 ((bx1-ax1)*(by1-by2) - (by1-ay1)*(bx1-bx2))
* 
*   D2 ((ax2-ax1)*(by1-ay1) - (ay2-ay1)*(bx1-ax1))
***********************************************************************/

int RandomWalk::G_intersect_line_segments (
     double ax1,double ay1, double ax2,double ay2,
     double bx1,double by1, double bx2,double by2,
     double *ra,double *rb,
     double *x,double *y)
{
	double D  = ((ax2-ax1)*(by1-by2) - (ay2-ay1)*(bx1-bx2));
	double D1 = ((bx1-ax1)*(by1-by2) - (by1-ay1)*(bx1-bx2));
	double D2 = ((ax2-ax1)*(by1-ay1) - (ay2-ay1)*(bx1-ax1));


	double d; 
    d = D;
 
    if (d) /* lines are not parallel */
    {
        *ra = D1/d;
        *rb = D2/d;
 
        *x = ax1 + (*ra) * (ax2 - ax1) ;
        *y = ay1 + (*ra) * (ay2 - ay1) ;
        return (*ra >= 0.0 && *ra <= 1.0 && *rb >= 0.0 && *rb <= 1.0);
    }
 
    if (D1 || D2) return -1;  /* lines are parallel, not colinear */
 
    if (ax1 > ax2)
    {
        SWAP (ax1, ax2)
    }
    if (bx1 > bx2)
    {
        SWAP (bx1, bx2)
    }
    if (ax1 > bx2) return -1;
    if (ax2 < bx1) return -1;

    /* there is overlap */
    if (ax1 == bx2)
    {
        *x = ax1;
        *y = ay1;
        return 1; /* at endpoints only */
    }
    if (ax2 == bx1)
    {
        *x = ax2;
        *y = ay2;
        return 1; /* at endpoints only */
    }
 
    return 2; /* colinear with overlap on an interval, not just a single point*/
}

/**************************************************************************
FEMLib-Method: 
Task: DoJointEffectOfElementInitially(void)
Programing: This function does make a choice for each particle
			that lies on a crossroad or a joint. The contribution is 
			determined by Fluid Momentum. Roulette Wheel Selection (RWE)
			determines which brach the particle continue to travel.
02/2006 PCH 
**************************************************************************/
void RandomWalk::DoJointEffectOfElementInitially(void)
{
	// Get the mesh first
	CFEMesh* m_msh = fem_msh_vector[0];  // Something must be done later on here.

	// Looping all over the particles to have a choice which plane to go.
	// Because all of the particles are on the joint initially.
	for(int p=0; p< m_msh->PT->numOfParticles; ++p)
	{
		// Mount the element fromthe first particle from particles initially
		int eleIdx = m_msh->PT->X[p].Now.elementIndex;
		CElem* theEle = m_msh->ele_vector[eleIdx];
		// Let's get the number of edges in the element and mount them
		int numOfEdgeIntheElement = theEle->GetEdgesNumber();
		vec<CEdge*> theEdges(numOfEdgeIntheElement);
		theEle->GetEdges(theEdges);
		CEdge* theJoint = NULL;

		// Now, 1. find the joint out of theses edges
		for(int i=0; i<numOfEdgeIntheElement; ++i)
		{
			// Is this a joint?
			if(theEdges[i]->GetJoint() == 1)
				theJoint = theEdges[i];	
		}

		// 2. Get multiple planes out of the joint
		// Now we need one of crossroad from the joint
		// Get the nodes of the edge
		vec<CNode*> theNodes(3);
		theJoint->GetNodes(theNodes);
		// I will use the first node of the joint as a crossroad
		CNode* crossnode = theNodes[0];	
		// Let's mount the crossroad class
		CrossRoad* crossroad = NULL;
		for(int i=0; i < (int)(m_msh->fm_pcs->crossroads.size()); ++i)
		{
			if( m_msh->fm_pcs->crossroads[i]->Index == crossnode->GetIndex() )
				crossroad = m_msh->fm_pcs->crossroads[i];
		}
		// Let's get the contribution of each connected plane.
		double chances[100];	// I just set 100 as a maximum number of 
							// connected planes.
		for(int i=0; i<crossroad->numOfThePlanes; ++i)
			chances[i] = crossroad->plane[i].ratio;

		// 3. Roulette Wheel Selection
		int whichWay = RouletteWheelSelection(chances, crossroad->numOfThePlanes);
		m_msh->PT->X[p].Now.elementIndex = m_msh->PT->X[p].Past.elementIndex =
			crossroad->plane[whichWay].eleIndex;
	}
	

}

/************************************************************************** 
Task: ToTheXYPlane(CElem* E, double* X)
Programing: This function rotate-transforms the vector to be on the xy plane
02/2006 PCH 
**************************************************************************/
void RandomWalk::ToTheXYPlane(CElem* E, double* X)
{
	double x[3], xx[3];

	// Get the norm of the element plane and do some initialization
	for(int k=0; k<3; ++k)
		x[k] = xx[k] = 0.0;

	double alpha = E->GetAngle(0);
	double beta = E->GetAngle(1);
	// Let's rotate the original Enorm to the BB coordinate system 
	// along the y axis
	x[0] = cos(alpha)*X[0] + sin(alpha)*X[2];
	x[1] = X[1];
	x[2] = -sin(alpha)*X[0] + cos(alpha)*X[2];	
	// Let's rotate the BB coordinate system to the BBB coordinate system
	// along the x axis
	xx[0] = x[0];
	xx[1] = cos(beta)*x[1] - sin(beta)*x[2];
	xx[2] = sin(beta)*x[1] + cos(beta)*x[2];

	for(int i=0; i<3; ++i) X[i] = xx[i];
	// Do translation along z'' axis.
//	X[2] -= E->GetAngle(2);
}

void RandomWalk::ToTheXYPlane(int idx, double* X)
{
	CFEMesh* m_msh = NULL;  
	if(fem_msh_vector.size()==0)
		return; //OK
	m_msh = fem_msh_vector[0];  
	CElem* E = m_msh->ele_vector[idx];

	double x[3], xx[3];

	// Get the norm of the element plane and do some initialization
	for(int k=0; k<3; ++k)
		x[k] = xx[k] = 0.0;

	double alpha = E->GetAngle(0);
	double beta = E->GetAngle(1);
	// Let's rotate the original Enorm to the BB coordinate system 
	// along the y axis
	x[0] = cos(alpha)*X[0] + sin(alpha)*X[2];
	x[1] = X[1];
	x[2] = -sin(alpha)*X[0] + cos(alpha)*X[2];	
	// Let's rotate the BB coordinate system to the BBB coordinate system
	// along the x axis
	xx[0] = x[0];
	xx[1] = cos(beta)*x[1] - sin(beta)*x[2];
	xx[2] = sin(beta)*x[1] + cos(beta)*x[2];

	for(int i=0; i<3; ++i) X[i] = xx[i];
	// Do translation along z'' axis.
//	X[2] -= E->GetAngle(2);
}

/************************************************************************** 
Task: ToTheRealPlane(CElem* E, double* X)
Programing: This function transform the vector on the xy plane to the 
			original plane of the element in 3D.
02/2006 PCH 
**************************************************************************/
void RandomWalk::ToTheRealPlane(CElem* E, double* X)
{	
	double x[3], xx[3];

	// Get the norm of the element plane and do some initialization
	for(int k=0; k<3; ++k)
		x[k] = xx[k] = 0.0;

	double alpha = E->GetAngle(0);
	double beta = E->GetAngle(1);
	// Let's rotate the original Enorm to the BB coordinate system 
	// along the y axis
	x[0] = cos(alpha)*X[0] - sin(alpha)*X[2];
	x[1] = X[1];
	x[2] = sin(alpha)*X[0] + cos(alpha)*X[2];	
	// Let's rotate the BB coordinate system to the BBB coordinate system
	// along the x axis
	xx[0] = x[0];
	xx[1] = cos(beta)*x[1] + sin(beta)*x[2];
	xx[2] = -sin(beta)*x[1] + cos(beta)*x[2];

	for(int i=0; i<3; ++i) X[i] = xx[i];
	// Let's translate back to z axis.
//	X[2] += E->GetAngle(2);
}

void RandomWalk::ToTheRealPlane(int idx, double* X)
{
	CFEMesh* m_msh = NULL;  
	if(fem_msh_vector.size()==0)
		return; 
	m_msh = fem_msh_vector[0];  
	CElem* E = m_msh->ele_vector[idx];

	double x[3], xx[3];

	// Get the norm of the element plane and do some initialization
	for(int k=0; k<3; ++k)
		x[k] = xx[k] = 0.0;

	double alpha = E->GetAngle(0);
	double beta = E->GetAngle(1);
	// Let's rotate the original Enorm to the BB coordinate system 
	// along the y axis
	x[0] = cos(alpha)*X[0] - sin(alpha)*X[2];
	x[1] = X[1];
	x[2] = sin(alpha)*X[0] + cos(alpha)*X[2];	
	// Let's rotate the BB coordinate system to the BBB coordinate system
	// along the x axis
	xx[0] = x[0];
	xx[1] = cos(beta)*x[1] + sin(beta)*x[2];
	xx[2] = -sin(beta)*x[1] + cos(beta)*x[2];

	for(int i=0; i<3; ++i) X[i] = xx[i];
	// Let's translate back to z axis.
//	X[2] += E->GetAngle(2);
}

/************************************************************************** 
Task: SolveAnglesOfTheElment(CElem* E)
Programing: This function solves two angles for rotation transformation
02/2006 PCH 
**************************************************************************/
void RandomWalk::SolveAnglesOfTheElment(CElem* E)
{
	CFEMesh* m_msh = NULL;  
	if(fem_msh_vector.size()==0)
		return;
	m_msh = fem_msh_vector[0];  

	double Enorm[3];
    E->AllocateMeomoryforAngle(); //WW 
	// Get the norm of the element plane and do some initialization
	if(m_msh->GetCoordinateFlag() != 32)
	{
			Enorm[0] = 0.0; Enorm[1] = 0.0; Enorm[2] = 1.0;
	}
	else
		for(int k=0; k<3; ++k)
			Enorm[k] = E->getTransformTensor(k+6);

	// solve for two angles for two rotation transformation.
	// Solving alpha that will be used for rotation along y axis.
	double alpha = acos(Enorm[2]/sqrt(Enorm[0]*Enorm[0]+Enorm[2]*Enorm[2]));
	// The following if condition is required because
	// the acos function is not distintive in the case that Enorm[0]'s of  
	// the two planes are opposite each other. 
	if(Enorm[0] < 0.0)
		E->SetAngle(0, alpha );
	else
		E->SetAngle(0, alpha + 2.0*(PI - alpha) );
	
	// Solving beta that will be used for rotation along x axis
	double beta = 0.0, BB[3], TranZ;
	// Let's rotate the original Enorm to this coordinate system.
	BB[0] = cos(E->GetAngle(0))*Enorm[0] + sin(E->GetAngle(0))*Enorm[2];
	BB[1] = Enorm[1];
	BB[2] = -sin(E->GetAngle(0))*Enorm[0] + cos(E->GetAngle(0))*Enorm[2];	
	beta = atan(BB[1]/BB[2]);
	E->SetAngle(1, beta );

	// Solve for the translation.
	// I'll use the center of the element for this translation.
	double* center = E->GetGravityCenter();
	double x[3], xx[3];
	// Get the norm of the element plane and do some initialization
	for(int k=0; k<3; ++k)
		x[k] = xx[k] = 0.0;
	// Let's rotate the original Enorm to the BB coordinate system 
	// along the y axis
	x[0] = cos(E->GetAngle(0))*center[0] + sin(E->GetAngle(0))*center[2];
	x[1] = center[1];
	x[2] = -sin(E->GetAngle(0))*center[0] + cos(E->GetAngle(0))*center[2];	
	// Let's rotate the BB coordinate system to the BBB coordinate system
	// along the x axis
	xx[0] = x[0];
	xx[1] = cos(E->GetAngle(1))*x[1] - sin(E->GetAngle(1))*x[2];
	xx[2] = sin(E->GetAngle(1))*x[1] + cos(E->GetAngle(1))*x[2];
	TranZ = xx[2];
	E->SetAngle(2, TranZ);
}

/**************************************************************************
FEMLib-Method: 
Task: RouletteWheelSelection(double *chances, int numOfCases)
Programing: This function makes a choice by RWS that is based on 
			velocity contribution on each of the connected planes.
02/2006 PCH 
**************************************************************************/
int RandomWalk::RouletteWheelSelection(double *chances, int numOfCases)
{
	int whichOne = -1000; // Set it meaningless
	double* roulette;
	roulette = new double [numOfCases] ();

	MakeRoulette(chances, roulette, numOfCases);
	whichOne = Select(roulette, numOfCases);

	delete [] roulette;

	return whichOne; 
}

/**************************************************************************
FEMLib-Method: 
Task: MakeRoulette(double* fit, double* roulette, int numOfCases)
Programing: This function makes a roulette according to chances (fit)
02/2006 PCH 
**************************************************************************/
void RandomWalk::MakeRoulette(double* fit, double* roulette, int numOfCases)
{
	double* pi = NULL; double* fitProbability = NULL;
	double fitTotal = 0.0, ProbTotal = 0.0;

	// Create memory for these two arrays dynamically
	pi = new double [numOfCases] (); fitProbability = new double [numOfCases] ();

	for(int i=0; i<numOfCases; ++i)
	{
		// Function modification can be done here. 
		pi[i] = 1./exp(-fit[i]);
		fitTotal += pi[i];
	}
	
	// Making Roulette
	for(int i=0; i<numOfCases; ++i)
	{
		fitProbability[i] = pi[i] / fitTotal;
		ProbTotal += fitProbability[i];
		roulette[i] = ProbTotal;
	}

	delete [] pi; delete [] fitProbability;	
}

/**************************************************************************
FEMLib-Method: 
Task: Select(double* roulette)
Programing: This function select a choice out of the roulette
02/2006 PCH 
**************************************************************************/
int RandomWalk::Select(double* roulette, int numOfCases)
{
	double probability;
	
	probability = randomZeroToOne();
	for(int i=0; i<numOfCases; ++i)
		if(probability < roulette[i])
			return (i);
}

/**************************************************************************
FEMLib-Method: 
Task: Random Walk read function
Programing:
09/2005 PCH Destruct before read
**************************************************************************/
void PCTRead(string file_base_name)
{
    CFEMesh* m_msh = NULL;
  
	if(fem_msh_vector.size()==0)
		return; //OK
    
	m_msh = fem_msh_vector[0];  // Something must be done later on here.

    // File handling
    string pct_file_name;
    ios::pos_type position;
    pct_file_name = file_base_name + PCT_FILE_EXTENSION;

    ifstream pct_file (pct_file_name.data(),ios::in);
    if(!pct_file.good()) //WW
        return;
    int End = 1;
    string strbuffer;    
    RandomWalk* RW = NULL;
#ifdef RANDOM_WALK	
	m_msh->PT = new RandomWalk(); //PCH
    RW = m_msh->PT;
#endif
 
    while(End)
    {
        // Later on from this line, I can put which mesh I am dealing with. 
        //getline(pct_file, strbuffer); 
        pct_file>>RW->numOfParticles>>ws;
        // Now allocate memory
        RW->CreateParticles(RW->numOfParticles);
        for(int i=0; i< RW->numOfParticles; ++i)
        {
            // Assign the number to the particle
            int idx = 0;
            double x = 0.0, y=0.0, z=0.0;
            pct_file>>idx>>x>>y>>z>>ws;
            RW->X[i].Past.elementIndex = RW->X[i].Now.elementIndex = idx;
            RW->X[i].Past.x = RW->X[i].Now.x = x;
            RW->X[i].Past.y = RW->X[i].Now.y = y;
            RW->X[i].Past.z = RW->X[i].Now.z = z;
        }

        End = 0;
    }   
}
/**************************************************************************
ROCKFLOW - Funktion: DATWriteFile
Task: Write PCT file
Programing:
09/2005   PCH   Implementation
**************************************************************************/
void DATWritePCTFile(const char *file_name)
{
    FILE *pct_file = NULL;
    char pct_file_name[MAX_ZEILE];

    CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0];  // Something must be done later on here.

    RandomWalk* RW = NULL;
#ifdef RANDOM_WALK	//WW
    RW = m_msh->PT;
#endif
    
    sprintf(pct_file_name,"%s.%s",file_name,"pct");
    pct_file = fopen(pct_file_name,"w+t");
    
    fprintf(pct_file, "%d\n", RW->numOfParticles);
    for(int i=0; i< RW->numOfParticles; ++i)
    {
        fprintf(pct_file, "%d %17.12e %17.12e %17.12e\n",
            RW->X[i].Now.elementIndex, RW->X[i].Now.x, RW->X[i].Now.y, RW->X[i].Now.z);
    }

    // Let's close it, now
    fclose(pct_file);

}

