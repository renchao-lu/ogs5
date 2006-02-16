// Construction/destruction
CRFProcess::CRFProcess(void)
CRFProcess::~CRFProcess(void)
void CRFProcess::PCSCreateProcess()
void PCSDestroyAllProcesses(void)
void PCSRead(string file_base_name)
ios::pos_type CRFProcess::Read(ifstream *pcs_file)
// Access to PCS
CRFProcess *CRFProcess::Get(string name)
CRFProcess *CRFProcess::GetProcessByFunctionName(char *name)
CRFProcess *CRFProcess::GetProcessByNumber(int number)
// Configuration 1 - NOD
void CRFProcess::PCSConfigProcess(void)
void CRFProcess::PCSConfigNODValues(void)
void CRFProcess::PCSCreateNODValues(void)
int PCSGetNODValueIndex(string name,int timelevel)
double PCSGetNODValue(long node,char* name,int timelevel)
// Configuration 2 - ELE
void CRFProcess::PCSConfigELEValues(void)
void CRFProcess::PCSCreateELEValues(void)
void CRFProcess::PCSCreateELEGPValues(void)
int PCSGetELEValueIndexNew(char *name)
double PCSGetELEValue(long index,double*gp,double theta,string nod_fct_name)
// Configuration 3 - ELE matrices
void CRFProcess::PCSCreateELEMatricesPointer(void)
??? void PCSConfigELEMatricesXXX(int pcs_type_number)
// Execution
double CRFProcess::PCSExecuteProcess()
void CRFProcess::PCSInitEQS()
void CRFProcess::PCSCalculateElementMatrices(void) 
void CRFProcess::DomainDecomposition()
void CRFProcess::PCSAssembleSystemMatrixNew(void) 
void CRFProcess::PCSIncorporateBoundaryConditions(const double Scaling)
void CRFProcess::PCSIncorporateSourceTerms(const double Scaling)
int CRFProcess::PCSExecuteLinearSolver(void)
// Specials
void PCSRestart(void)
void RelocateDeformationProcess(CRFProcess *m_pcs)
void CRFProcess::CreateFDMProcess()
void CRFProcess::PCSMoveNOD(void) 
string PCSProblemType()
// ReMove site