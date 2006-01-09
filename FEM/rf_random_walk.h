/**************************************************************************
FEMLib-Object: 
Task: Random Walk - an alternative for FDM or FEM of transport equation
Programing:
07/2005 PCH Implementation
**************************************************************************/
#ifndef rf_random_walk_INC

#define rf_random_walk_INC

// MATH
#include <math.h>

// C++ STL
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include "rf_pcs.h"
#include "mathlib.h"

#include "rf_mmp_new.h"

class Particle
{
public:
    // Position Vector
    double x;
    double y;
    double z;

    // Velocity Vector
    double Vx;
    double Vy;
    double Vz;

    // Time
    double t;

    // the element it belongs to
    int elementIndex;

    // Constructor
    Particle(void);

    // Some operator overloading
    Particle operator=(const Particle& B) const
    {
        Particle A;
        A.x = B.x; A.y = B.y; A.z = B.z;
        A.Vx = B.Vx; A.Vy = B.Vy; A.Vz = B.Vz;
        A.t = B.t; A.elementIndex = B.elementIndex;

        return A;
    };
};


class Trace
{
public:
    // I am only bookkeeping the current and future positions 
    // to save memory.
    Particle Past;
    Particle Now;

// I am going to use the system default constructor and destructor.
};

class RandomWalk
{
public:  
	RandomWalk(void);
	~RandomWalk(void);
 
    // This will be determined from user's input
    // This may be a global variable somewhere in application-level later.
    // For now, I just created here. 
    int numOfParticles;  

    // This will keep current and future particles only.
    Trace* X;   

    void CreateParticles(int HowManyParticles);
    void AssignParticlesToElements(void);
    double Marsaglia(void); // N(0,1) sample generator
    int IsTheParticleInThisElement(Particle* A, CElem* m_ele);
    void InterpolateVelocityOfTheParticle(Particle* A, CElem* m_ele);
    void GetVelocityFromTriangle(Particle* A, CElem* m_ele, double* x1buff,
        double* x2buff,  double* x3buff);
    void GetVelocityFromTetrahedra(Particle* A, CElem* m_ele, double* x1buff,
        double* x2buff,  double* x3buff, double* x4buff);
    void IdentifyCurrentParticle(double dt);
    double randomMinusOneToOne(void);   // create uniform random number between -1 and 1
    double randomZeroToOne(void);   // create uniform random number between -1 and 1

    void AdvanceParticles(double dt);
    int SearchTheElementThatHasTheDisplacedParticle(Particle* A, CElem* m_ele);
    void RandomlyDriftAway(Particle* A, double* V, double dt, double* delta);

protected:
	FiniteElement::CFiniteElementStd *fem;


private:    
	CRFProcess* m_pcs;
   
    double ComputeVolume(Particle* A, CElem* m_ele);
    double ComputeVolume(Particle* A, Particle* element, CElem* m_ele);
    void CopyParticleCoordToArray(Particle* A, double* x1buff, double* x2buff, double* x3buff, double* x4buff);
};


extern void PCTRead(string);
//extern void DATWritePCTFile(const char *file_name);
#endif




