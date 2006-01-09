/**************************************************************************
Class: RandomWalk
Task: Random Walk - an alternative for FDM or FEM of transport equation
Programing:
07/2005 PCH Implementation
**************************************************************************/
#include <cstdlib>
#include <cmath>

#include "stdafx.h" //MFC
#include "rf_random_walk.h"
#include "elements.h"

// C++ STL
#include <iostream>
using namespace std;

#define PCT_FILE_EXTENSION ".pct"
#define OUTSIDEOFDOMAIN -10


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
    fem = NULL;

    // This is going to be reset by user input.
    // Further, used for allocating dynamic memory.
    numOfParticles = 0;
    X = NULL;

    // To produce a different pseudo-random series each time your program is run.
    srand((unsigned int)time(0)); //OK
}


// Constructor
Particle::Particle(void)
{
    // Position Vector
    x = y = z = 0.0;

    // Velocity Vector
    Vx = Vy = Vz = 0.0;
   
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
        PieceOfVolume = new double[2];
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
            abort();

        delete Tri1;
        delete Tri2;
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

        delete Tet1; delete Tet2; delete Tet3; delete Tet4; delete Tet5; delete Tet6;   
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

        delete Pri1; delete Pri2; delete Pri3; 
       
    }

    // Release the memory
    delete PieceOfVolume;


    
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
    PieceOfVolume = new double[3];

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
}


void RandomWalk::GetVelocityFromTetrahedra(Particle* A, CElem* m_ele, double* x1buff,
     double* x2buff,  double* x3buff, double* x4buff)
{
    double volume = 0.0;
    double A2buff[3];
    double* PieceOfVolume = NULL;
    PieceOfVolume = new double[4];

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
    X = new Trace[numOfParticles];
}


/**************************************************************************
Class: RandomWalk
Task: This function assign the number of particles in the elements picked
      randomly. It does not matter with element type or a number of elements
Programing:
09/2005 PCH Implementation
last modification:
**************************************************************************/
void RandomWalk::AssignParticlesToElements(void)
{

}

/**************************************************************************
Class: RandomWalk
Task: This function solves current information for particles 
      based on the information given previously.
      This includes advection and dispersion together.
Programing:
08/2005 PCH Implementation
last modification:
**************************************************************************/
void RandomWalk::IdentifyCurrentParticle(double dt)
{
dt = dt; //OK
    // Let's loop over the whole particles
    for(int i=0; i< numOfParticles; ++i)
    {
        //X->Now.x = X->Past.x + Ai
    }
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
        PieceOfVolume = new double[2];
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
        PieceOfVolume = new double[4];
        
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
        PieceOfVolume = new double[12];
        
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
        PieceOfVolume = new double[3];

        PieceOfVolume[0] = ComputeDetTri(x1buff, x2buff, A2buff) ;
        PieceOfVolume[1] = ComputeDetTri(x2buff, x3buff, A2buff) ;
        PieceOfVolume[2] = ComputeDetTri(x3buff, x1buff, A2buff) ;

        for(int i=0; i<3; ++i)
            volume += PieceOfVolume[i];     
    }
    //TETRAHEDRAS = 5 
	if (m_ele->GetElementType() == 5)
	{
        PieceOfVolume = new double[4];
        
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
        PieceOfVolume = new double[8]; 

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
    delete PieceOfVolume;

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

//OK    CNode* node = NULL;

    double A2buff[3];

    A2buff[0] = A->x; A2buff[1] = A->y; A2buff[2] = A->z;

    x1buff[0] = element[0].x; x1buff[1] = element[0].y; x1buff[2] = element[0].z; 
    x2buff[0] = element[1].x; x2buff[1] = element[1].y; x2buff[2] = element[1].z;
    x3buff[0] = element[2].x; x3buff[1] = element[2].y; x3buff[2] = element[2].z;
    
  
    //TRIANGLES = 4, RECTANGLE = 2
    int eleType = m_ele->GetElementType(); 
	if (eleType == 4 || eleType == 2)
	{
        PieceOfVolume = new double[3];

        PieceOfVolume[0] = ComputeDetTri(x1buff, x2buff, A2buff) ;
        PieceOfVolume[1] = ComputeDetTri(x2buff, x3buff, A2buff) ;
        PieceOfVolume[2] = ComputeDetTri(x3buff, x1buff, A2buff) ;

        for(int i=0; i<3; ++i)
            volume += PieceOfVolume[i];     
    }
    //TETRAHEDRAS = 5, HEXAHEDRA = 3, PRISM = 6
	else if (eleType == 5 || eleType == 3 || eleType == 6)
	{
        PieceOfVolume = new double[4];
        
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
    delete PieceOfVolume;

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
        double V[3];

        // Solve velocity of the particle "NOW" and assign it;
        CFEMesh* m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
        CElem* m_ele = m_msh->ele_vector[X[i].Now.elementIndex];
        InterpolateVelocityOfTheParticle(&(X[i].Now), m_ele);
        V[0] = X[i].Now.Vx; V[1] = X[i].Now.Vy; V[2] = X[i].Now.Vz;
   
        double delta[3];
        // Compute the random drift for the particle by 
        // solving the dispersion tensor at the particle location
        RandomlyDriftAway(&(X[i].Now), V, dt, delta);
            
        // Now we need the dispersion tensor at the position 
        // where the particle drifted randomly by delta Of X.
        // For this I need velocity at the displaced position meaning some smart searching technique.
        // First let's assign one particle for the displaced position.
        Particle Y; // the displaced particle
        Y.x = X[i].Now.x + delta[0]; Y.y = X[i].Now.y + delta[1]; Y.z = X[i].Now.z + delta[2];
        // Next I need to know which element has this particle.
        Y.elementIndex = SearchTheElementThatHasTheDisplacedParticle(&Y, m_ele);
        // Still, I need to check if the displaced particle is outside of the domain
        // If the diplaced particle is still in the domain,
        if(Y.elementIndex != OUTSIDEOFDOMAIN)
        {
            // Solve the random drift for the displaced location
            RandomlyDriftAway(&Y, V, dt, delta);

            // Let's record the current to the past
            X[i].Past = X[i].Now; 
            // Now advance the particle by advection and dispersion
            // Advection: V[i]*dt, Dispersion: delta[i];
            X[i].Now.x += V[0]*dt + delta[0]; 
            X[i].Now.y += V[1]*dt + delta[1];
            X[i].Now.z += V[2]*dt + delta[2];
            X[i].Now.t += dt;   // Update the current time
        }
        // If the displace particle is outside of the domain
        else
        {
            // I will do nothing here. Thus, a condition to check 
            // if elementIndex == -10 should be accompanied in the functions.
            // This also includes the display in OpenGL.
            // This makes sense because the particle once gone outside 
            // never comes back in.       
        }    
    }
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
            
            if(IsTheParticleInThisElement(A, thisNeighbor) == 1)
                return thisNeighbor->GetIndex();
        }
        // If the code pases the following loop, it means I am not lucky in this neighbor search.
        CFEMesh* m_msh = fem_msh_vector[0];      // This is because FEM is not executed. Something should be done later.
        long numberOfElements = (long)m_msh->ele_vector.size(); //OK
        for(long i=0; i< numberOfElements; ++i)
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
void RandomWalk::RandomlyDriftAway(Particle* A, double* V, double dt, double* delta)
{
    // To extract dispersivities from material properties    
    CMediumProperties *m_mat_mp = NULL;
    double alphaL = 0.0, alphaT = 0.0;
    double D[9];

    // Extract the dispersivities from the group that the particle belongs
    int group = ElGetElementGroupNumber(A->elementIndex);
    m_mat_mp = mmp_vector[group];
    alphaL = m_mat_mp->mass_dispersion_longitudinal;
    alphaT = m_mat_mp->mass_dispersion_transverse;

    // Just solve for the magnitude of the velocity to compute the dispersion tensor
    double Vmagnitude = sqrt(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);

    // Compute the dispersion tensor at the particle location
    D[0] = (alphaT*(V[1]*V[1]+ V[2]*V[2]) + alphaL*V[0]*V[0]) / Vmagnitude; // Dxx
    D[1] = D[3] = (alphaL- alphaT)*V[0]*V[1]/Vmagnitude;    // Dxy = Dyz
    D[2] = D[6] = (alphaL- alphaT)*V[0]*V[2]/Vmagnitude;    // Dxz = Dzx
    D[4] = (alphaT*(V[0]*V[0]+ V[2]*V[2]) + alphaL*V[1]*V[1]) / Vmagnitude; // Dyy
    D[5] = D[7] = (alphaL- alphaT)*V[1]*V[2]/Vmagnitude;    // Dyz = Dzy
    D[8] = (alphaT*(V[0]*V[0]+ V[1]*V[1]) + alphaL*V[2]*V[2]) / Vmagnitude; // Dzz

    // Let's generate three random components N(0,1) and use it to compute deltaOfX
    double Z[3];
    Z[0] = Marsaglia(); Z[1] = Marsaglia(); Z[2] = Marsaglia();
    delta[0] = sqrt(2.0*D[0]*dt) * Z[0] + sqrt(2.0*D[1]*dt) * Z[1] + sqrt(2.0*D[2]*dt) * Z[2];
    delta[1] = sqrt(2.0*D[3]*dt) * Z[0] + sqrt(2.0*D[4]*dt) * Z[1] + sqrt(2.0*D[5]*dt) * Z[2];
    delta[2] = sqrt(2.0*D[6]*dt) * Z[0] + sqrt(2.0*D[7]*dt) * Z[1] + sqrt(2.0*D[8]*dt) * Z[2];
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
    m_msh = fem_msh_vector[0];  // Something must be done later on here.

    // File handling
    string pct_file_name;
    ios::pos_type position;
    pct_file_name = file_base_name + PCT_FILE_EXTENSION;

    ifstream pct_file (pct_file_name.data(),ios::in);

    int End = 1;
    string strbuffer;    
    RandomWalk* RW = NULL;
    RW = m_msh->PT;
 
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
    RW = m_msh->PT;
    
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


