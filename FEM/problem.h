/*
  Class to define a problem to be solved.
  Programming WW 08.07.2008
*/
#if defined(PROBLEM_CLASS)
#ifndef problem_INC
#define problem_INC
#include <vector>
class CRFProcess;
using namespace std;
//---------------------------------------------------------------------
//Pointers to member functions
class Problem;
typedef  double (Problem::*ProblemMemFn)(void); 
#define Call_Member_FN(object,ptrToMember)  ((object)->*(ptrToMember)) 
//---------------------------------------------------------------------
class Problem
{
  public:
   Problem(char* filename = NULL);
   ~Problem();
   void Euler_TimeDiscretize();
   void RosenBrock_TimeDiscretize() {};
   //
   void SetActiveProcesses();
   void PCSRestart();
   //
   bool CouplingLoop();
   void PostCouplingLoop();
   // Copy u_n for auto time stepping
   double* GetBufferArray() {return buffer_array;};
  private:
   // Time:
   double start_time;
  public: //OK
   double end_time;
   double current_time;
  private:
   double *buffer_array;
   int step_control_type;
   // Mixed time step WW
  public: //OK
   double dt0; // Save the original time step size
  private:
   // Controls
   int loop_index;
   int max_coupling_iterations;
  public: //OK
   int max_time_steps;
  private:
   double coupling_tolerance;
   //
   double TolCoupledF;
   double pcs_flow_error;
   double pcs_flow_error0;
   double pcs_coupling_error;
   int lop_coupling_iterations;
   bool CalcVelocities; 
   bool conducted; 
   // Print flag
  public: //OK
   bool print_result;
  private:
   // Processes
  public: //OK
   vector<CRFProcess*> total_processes;
  private:
   vector<CRFProcess*> transport_processes;
   vector<CRFProcess*> multiphase_processes;
   ProblemMemFn *active_processes;
  public: //OK
   vector<int> active_process_index;
  private:
   vector<int> coupled_process_index;
   // 
   bool *exe_flag;
   inline int AssignProcessIndex(CRFProcess *m_pcs, bool activefunc = true); 
   //
   void PCSCreate();
   // Perform processes:
   inline double LiquidFlow();
   inline double RichardsFlow();
   inline double TwoPhaseFlow(); 
   inline double MultiPhaseFlow();
   inline double GroundWaterFlow();
   inline double ComponentalFlow(); 
   inline double OverlandFlow();
   inline double AirFlow();
   inline double HeatTransport();
   inline double FluidMomentum();
   inline double RandomWalker();
   inline double MassTrasport();
   inline double Deformation();
   // Accessory 
   void LOPExecuteRegionalRichardsFlow(CRFProcess*m_pcs_global);
   void LOPCalcELEResultants();
   inline void ASMCalcNodeWDepth(CRFProcess *m_pcs);
   void PCSCalcSecondaryVariables();
   bool Check(); //OK
};
extern Problem *aproblem;
extern bool MODCreate(); //OK
#endif
#endif //PROBLEM_CLASS
