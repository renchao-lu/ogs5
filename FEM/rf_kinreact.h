/**************************************************************************
rf_kinreact.cpp

                                    KINETIC REACTIONS

FEMLib-Object KinReact

Programming:
01/2004    Dirk Schäfer       Original IMplementation
02/2006    Sebastian Bauer    Adaption to C++ Class structure, new FEM concept

***************************************************************************/
#ifndef rf_kinreact_INC
#define rf_kinreact_INC
// C++ STL
#include <fstream>
#include <string>
#include <vector>
using namespace std;

/* residual for linearisation of critical functions */
#define residual 1.E-20
#define maxMonod 5
#define maxInhibition 5
#define maxBioReactions 30
#define maxNumber_of_Components 30


#define KRC_FILE_EXTENSION ".krc"

/* New class KinReaction: contains the kinetic reactions and all necessary data structures for them */
// C++ Class Monodsubstruct
class MonodSubstruct {
private:
public:
	string species ; // Name of species
	int speciesnumber; // number of species;
	double concentration; //Monod concentration
	double order ; // Order of monod term
 int isotopecouplenumber; // CB isotope fractionation : specis number of isotope partner
 // CB for Threshhold terms
 bool threshhold;
 double threshConc;
 double threshOrder;
	MonodSubstruct(void);
	~MonodSubstruct(void);
};

// C++ Class CKinReact
class CKinReact{
    private:

    public:
    CKinReact(void); // Constructor
    ~CKinReact(void); // Destructor
    
	string	name;				/* name of reaction */
	string	type;					/* type of reaction: monod, exchange, NAPLdissolution, ... */
    int		number;                     /* counter */
	int		number_reactionpartner;	/* Number of chemical species involved in reaction */
	vector <string> reactionpartner;	/* all names of reaction partners stored here */
	vector <double> stochmet;			/* stochiometric coefficients for each reactionpartner stored here */
	double	rateconstant;		/* rateconstant */
	double	rateorder;			/* order of reaction */
	int		number_monod;			/* Number of Monod terms */
	int		number_inhibit;			/* Number of inhibition terms */
	int		number_production;		/* number of production terms */
	int		number_isotope_couples;		/* number of production terms */
	vector <MonodSubstruct*>  monod;		/* saves monod concentrations and names of species */
	vector <MonodSubstruct*>  inhibit;		/* saves inhibit concentrations and names of species */
	vector <MonodSubstruct*>  production;	/* saves production concentrations, orders and names of species */
    int grow; /* growth or no growth */
    string	bacteria_name;
    int		bacteria_number;
    vector <double>	ProductionStoch; // stochiometry of reaction
//    vector <double>	ProductionStoch2; // stochiometry of reaction - short version
	vector <MonodSubstruct*> ProdStochhelp; // store input values
    //CB Isotope fractionation  
    string Isotope_light;
    string Isotope_heavy;
string degType;
double isoenfac ;
//CB Not this particular reaction on specified GEO-Objects; Data structures
 vector <string> NotThisReactGeoName;
	vector <string> NotThisReactGeoType;
	vector <bool> switched_off_node;

	// exchange data
	vector <string>	ex_species_names;
	vector <int>	ex_species;
	vector <double>	ex_param;
	int		exSurfaceID;
	string exType ;					/* sorption type: linear, langmuir, exchange */

    //#ds NAPLdissolution data
	string  blob_name;              /* name of blob-class */
	int     blob_ID;                /* id number of blobs where the NAPL phase resides */
	double  Csat_pure;              /* maximum solubility of the pure NAPL phase */
	double  current_Csat;            /* current solubility after considering Roult's law, interally calculated */
	double  Density_NAPL;           /* density of the pure NAPL phase */
//	double  ConversionFactor;       /* factor to convert concentrations to mol/kg */
    //SB speed-up flags
	int typeflag_monod;		/* set to 1 if reaction is monod type */
	int typeflag_exchange;		/* set to 1 if reaction is exchange type */
	int typeflag_exchange_linear;		/* set to 1 if reaction is linear exchange type */
	int typeflag_exchange_langmuir;		/* set to 1 if reaction is langmuir exchange type */
	int typeflag_exchange_freundlich;		/* set to 1 if reaction is freundlich exchange type */
	int typeflag_napldissolution;		/* set to 1 if reaction is NAPL dissolution */
 int typeflag_iso_fract; /* set to 1 if reaction is isotope fractionation */

    /* Methods */
   bool Read(ifstream*); /* Class Read Function */
   void Write(ofstream*);       /* Class Write Function */
   void ReadReactionEquation(string); /* Read function for chemical equations */
   int CheckReactionDataConsistency(void); /* check data set */
   void TestWrite(void); // test output function

   double Monod(double, double, double, double); // CB isotope fractionation + higher order terms
   double Inhibition(double, double);
   double BacteriaGrowth ( int r, double *c, double sumX, int exclude );
   int	  GetPhase(int );
//   double GetPorosity( int comp, long index );
// CB replaced by
   double GetReferenceVolume( int comp, long index );
   double GetDensity( int comp, long index );
   double GetNodePoreVelocity( long node);
   double GetPhaseVolumeAtNode(long node, double theta, int phase);
   // CB 19/10/09
   long currentnode; // CB 19/10/09 This is eclusively for Brand model to allow porosity in Inhibition constant calculation
};


//#ds Class for blob properties
class CKinBlob {
private:

public:

	string	name;				            /* name of blob-class */
	double  d50;                            /* average diameter of sediment grains */
	double  Sh_factor;                      /* initial factor for calculation of Sherwood number */
	double  Re_expo;                        /* exponent of Reynolds number for calculation of Sherwood number */
	double  Sc_expo;                        /* exponent of Schmidt number for calculation of Sherwood number */
	double  Geometry_expo;                  /* exponent of relative volume for calculation of present interfacial area */
    double  Mass;                           /* current total Mass of blob, internally calculated for one node */
	double  Volume;                         /* current total Volume of blob, internally calculated for one node */
    double  Masstransfer_k;                 /* current Mass transfer coefficient, internally calculated for one node */
	double  current_Interfacial_area;       /* current Interfacial Area, internally calculated for one node only */ 
	vector <string> BlobGeoType;            /* definition of initial interfacial area based on Geo-objects */
    vector <string> BlobGeoName;            /* definition of initial interfacial area based on Geo-objects */
	vector <double> Area_Value;             /* initial value within the Geo-object */
    vector <double> Interfacial_area;       /* initial interfacial area, interfacial area of last iteration */

	CKinBlob(void);                 
	~CKinBlob(void);               
	bool Read(ifstream*);     /* Class Read Function #ds muss noch erstellt werden*/
    void Write(ofstream*);   /* Class Write Function #ds muss noch erstellt werden*/
	void TestWrite(void);
};


class CKinReactData{
    private:

    public:
    
    /* Data */
	int		SolverType;
	double	relErrorTolerance;
	double	minTimestep;
	double  initialTimestep;
    double  usedt;
	int		NumberReactions;
	int		NumberLinear;
	int		NumberLangmuir;
	int		NumberFreundlich;
	int		NumberMonod;
	int     NumberNAPLdissolution;
	

	// biodeg data
	double	maxBacteriaCapacity;
    vector <int> is_a_bacterium;
//	vector <int> is_a_bacterium2; // short version
	// exchange data
	int maxSurfaces;
	vector <double>	exSurface;
	// output flag
    bool testoutput;
	//index vector for shortening vectors c in kinetic calculations (omitting nonreacting species)
//	vector <int> sp_index;
//	int kr_active_species;
	vector <int> sp_pcsind;
	vector <int> sp_varind;
    
	// No reactions on specified GEO-Objects; Data structures
 vector <string> NoReactGeoName;
	vector <string> NoReactGeoType;
	vector <bool> is_a_CCBC;

 // CB ReactDeact no reaction switch
 bool ReactDeactFlag;        // method flag
 int ReactDeactPlotFlag;    // flag for tecplot plots of flags each timestep   
 double ReactDeactEpsilon;   // treshhold
 vector <bool> ReactDeact;   // flags for individual nodes
 vector <double> React_dCdT; // Sum of reaction rates for individual nodes
 vector <vector<int>> ReactNeighborhood; // node indices of local neighborhood around individual nodes
 int ReactDeactMode;

 bool debugoutflag;
 string debugoutfilename;
 ofstream debugoutstr;

	vector <double> node_foc;

 /* Methods */
 CKinReactData(void);
 ~CKinReactData(void);
 bool Read(ifstream*);     /* Class Read Function */
 void Write(ofstream*);   /* Class Write Function */
	void TestWrite(void);
 void ExecuteKinReact(void);
 void Biodegradation( long node, double eps, double hmin, double *usedtneu, int *nok, int *nbad);

 // CB ReactDeact
 void ReactionDeactivation(long);  // Sets nodes active / inactive
 void ReactDeactPlotFlagsToTec();
 void ReactDeactSetOldReactionTerms(long nonodes);

 double **concentrationmatrix;
 void Aromaticum(long nonodes);

};

extern vector <CKinReact*> KinReact_vector; // declare extern instance of class CKinReact
extern vector <CKinReactData*> KinReactData_vector; // declare extern instance of class CKinReact
extern vector <CKinBlob*> KinBlob_vector; // declare extern instance of class Blob

extern bool KRRead(string);
extern bool KRWrite(string);
extern void KRCDelete(void);
extern void KRConfig(void);
extern void KBlobConfig(void);  /* configure Blob-Object */
extern void KBlobCheck(void);   /* check Blob-Object for input errors */
extern bool KNaplDissCheck(void);   /* CB check if NAPL dissolution is modeled */




/* Externe Subroutine-Deklarationen fuer Bulirsch-Stoer Gleichungslöser */

extern void odeint(double ystart[], int nvar, double x1, double x2, double eps, double h1,
	double hmin, double *nexth, int *nok, int *nbad,
	void (*derivs)(double, double [], double [], int, long),
	void (*stifbs)(double [], double [], int, double *, double, double, double [],
	double *, double *, void (*)(double, double [], double [], int, long), long), long);

extern void stifbs(double y[], double dydx[], int nv, double *xx, double htry, double eps,
	double yscal[], double *hdid, double *hnext,
	void (*derivs)(double, double [], double [], int, long), long);

extern double *dvector(long nl, long nh);
extern void free_dvector(double *v, long nl, long nh);



/* interne Deklarationen */
extern void		ExecuteKineticReactions();
extern double	TBCGetExchange(long node, int sp);
extern void		derivs(double x, double y[], double dydx[], int n, long node);
extern void		jacobn(double x, double y[], double dfdx[], double **dfdy, int n, long node);


#endif
