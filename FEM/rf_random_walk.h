/**************************************************************************
FEMLib-Object: 
Task: Random Walk - an alternative for FDM or FEM of transport equation
Programing:
07/2005 PCH Implementation
**************************************************************************/

#include <math.h>
#include "mathlib.h"
#include "fem_ele.h"
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

	// Dispersion coefficient tensor
	double D[9];

	// Derivative of velcities
	double dVxdx;
	double dVydy;
	double dVzdz;


    // Time
    double t;

    // the element it belongs to
    int elementIndex;    

    // Constructor
    Particle(void);

    // Some operator overloading
	Particle& operator=(const Particle& B) 
    {
        x = B.x; y = B.y; z = B.z;
        Vx = B.Vx; Vy = B.Vy; Vz = B.Vz;
        t = B.t; elementIndex = B.elementIndex;
		dVxdx = B.dVxdx; dVydy = B.dVydy; dVzdz = B.dVzdz;

		for(int i=0; i<9; ++i)
			D[i] = B.D[i];

        return *this;
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
	int UniformOrNormal;

    // This will keep current and future particles only.
    Trace* X;   

    void CreateParticles(int HowManyParticles);
    double Marsaglia(void); // N(0,1) sample generator
    int IsTheParticleInThisElement(Particle* A, CElem* m_ele);
	int IsTheParticleInThisElement(Particle* A);
    void InterpolateVelocityOfTheParticle(Particle* A, CElem* m_ele);
	void InterpolateVelocityOfTheParticleByInverseDistance(Particle* A);
    void GetVelocityFromTriangle(Particle* A, CElem* m_ele, double* x1buff,
        double* x2buff,  double* x3buff);
    void GetVelocityFromTetrahedra(Particle* A, CElem* m_ele, double* x1buff,
        double* x2buff,  double* x3buff, double* x4buff);
    double randomMinusOneToOne(void);   // create uniform random number between -1 and 1
    double randomZeroToOne(void);   // create uniform random number between 0 and 1

    void AdvanceParticles(double dt);
	void AdvanceParticlesLaBolle(double dt);
	void AdvanceByAdvection(double dt);
	void AdvanceByAdvectionNDispersion(double dt);
	void AdvanceByAdvectionNDispersionSplitTime(double dt, int numOfSplit);
    int SearchTheElementThatHasTheDisplacedParticle(Particle* A, CElem* m_ele);
    void RandomlyDriftAway(Particle* A, double dt, double* delta, int type);
	int RandomWalkDrift(double* Z, int type);
    void TransformRandomDriftToTheElementPlane(CElem* m_ele, double* delta);
    void SetElementBasedConcentration(void);
	void SolveDispersionCoefficient(Particle* A);
	int SolveForDiffusionWithEdge(Particle* A, Particle* B, double* n);
	int SolveForAdvectionWithEdge(Particle* A, Particle* B, double* delta);
	int SolveForPureAdvectionWithEdge(Particle* A, Particle* B);
	int SolveForAdvectionNDispersionWithEdge(Particle* A, Particle* B);

	int SolveForTwoIntersectionsInTheElement(Particle* A, double* p1, double* p2, int axis);
	int SolveForDerivativeOfVelocity(Particle* A);
	int SolveForDisplacementByDerivativeOfDispersion(Particle* A, double* dD);
	double SolveDistanceBetweenTwoPoints(double* p1, double* p2);

	int GetTheElementOfTheParticleFromNeighbor(Particle* A);
	int GetTheElementOfTheParticleFromNeighborAggressive(Particle* A);

protected:
	FiniteElement::CFiniteElementStd *fem;

private:    
	CRFProcess* m_pcs;
    CFEMesh* m_msh;
   
    double ComputeVolume(Particle* A, CElem* m_ele);
    double ComputeVolume(Particle* A, Particle* element, CElem* m_ele);
    void CopyParticleCoordToArray(Particle* A, double* x1buff, double* x2buff, double* x3buff, double* x4buff);
	int G_intersect_line_segments (
		double ax1,double ay1, double ax2,double ay2,
		double bx1,double by1, double bx2,double by2,
		double *ra,double *rb,
		double *x,double *y);

	void ConcPTFile(const char *file_name);
};


extern void PCTRead(string);
extern void DATWritePCTFile(const char *file_name);





