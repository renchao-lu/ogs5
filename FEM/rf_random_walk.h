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
#include "rfmat_cp.h"

#define ALLOW_PARTICLES_GO_OUTSIDE

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

	// Konductivity
	double K;

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
	// particle identity
	int identity; 

    // Constructor
    Particle(void);

    // Some operator overloading
	Particle& operator=(const Particle& B) 
    {
        x = B.x; y = B.y; z = B.z;
        Vx = B.Vx; Vy = B.Vy; Vz = B.Vz;
		K=B.K;
        t = B.t; elementIndex = B.elementIndex;
		identity = B.identity;
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
	int RWPTMode;	// 0: Advection and dispersion for homogeneous media 
					// 1: Advection and dispersion for heterogeneous media
					// 2: Advection only for homogeneous media
					// 3: Advection only for heterogeneous media
					// 4: Dispersion only for homogeneous media
					// 5: Dispersion only for heterogeneous media
	int PURERWPT;	// 0: Defualt - Velocity solved by GeoSys
					// 1: Velocity fields on nodes are given in a separate file.

	double CurrentTime;

    // This will keep current and future particles only.
    Trace* X;   

    void CreateParticles(int HowManyParticles);
    double Marsaglia(void); // N(0,1) sample generator
	int IsTheParticleInThisElement(Particle* A);

	void InterpolateVelocityOfTheParticleByInverseDistance(Particle* A);
    
    double randomMinusOneToOne(void);   // create uniform random number between -1 and 1
    double randomZeroToOne(void);   // create uniform random number between 0 and 1


	void AdvanceParticlesLaBolle(double dt);

	void AdvanceToNextTimeStep(double dt);
	void AdvanceBySplitTime(double dt, int numOfSplit);
	void GetDisplacement(Particle* B, double* Z, double* V, double* dD, double time, double* dsp);

    void RandomlyDriftAway(Particle* A, double dt, double* delta, int type);
	int RandomWalkDrift(double* Z, int type);
    void SetElementBasedConcentration(double dt);
	void SolveDispersionCoefficient(Particle* A);
	
	int SolveForNextPosition(Particle* A, Particle* B);

	int SolveForTwoIntersectionsInTheElement(Particle* A, double* p1, double* p2, int axis);
	int SolveForDerivativeOfVelocity(Particle* A);
	int SolveForDisplacementByDerivativeOfDispersion(Particle* A, double* dD);
	double SolveDistanceBetweenTwoPoints(double* p1, double* p2);

	int GetTheElementOfTheParticleFromNeighbor(Particle* A);

	// Fracture Network
	void DoJointEffectOfElementInitially(void);
	void MakeRoulette(double* fit, double* roulette, int numOfCases);
	int Select(double* roulette, int numOfCases);
	int RouletteWheelSelection(double *chances, int numOfCases);

	// Transform coordinates
	void ToTheXYPlane(CElem* E, double* X);
	void ToTheXYPlane(int idx, double* X);
	void ToTheRealPlane(CElem* E, double* X);
	void ToTheRealPlane(int idx, double* X);
	void SolveAnglesOfTheElment(CElem* E);

	int ReadInVelocityFieldOnNodes(string file_base_name); // Read in velocity fields from an separate file

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





