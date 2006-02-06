/**************************************************************************
PARLib - Object: PAR
Task: class implementation
Programing:
07/2004 OK Implementation
last modified:
**************************************************************************/
#ifndef par_dd_INC

#define par_dd_INC

// C++ STL
#include <list>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
// FEMLib
#include "prototyp.h"
#include "rf_pcs.h"

//-----------------------------------------------
class CPARDomainElement
{
  private:
  public:
    long global_number;
    long *global_nodes; //Todo dynamical
    long nodes[3]; //Todo dynamical
};
//-----------------------------------------------
class CPARDomain
{
  private:
  public:
    int ID;
    //OK vector<CPARDomainElement*>elements;
    vector<CElem*>elements;
    vector<long>nodes_inner;
    vector<long>nodes_halo;
    vector<long>nodes;
    //?vector<double>matrix;
    // EQS
    LINEAR_SOLVER *eqs;
    LINEAR_SOLVER_PROPERTIES *lsp;
    char* lsp_name;
    // MSH
    CFEMesh* m_msh;
  public:
    CPARDomain(void);
    ~CPARDomain(void);
    ios::pos_type Read(ifstream*);
    void CreateNodes();
    void CreateElements();
    void CreateEQS();
    void CalcElementMatrices(CRFProcess*);
    void AssembleMatrix(CRFProcess*);
    void WriteMatrix();
    void SolveEQS();
    long GetDOMNode(long);
    int m_color[3]; //OK
    void WriteTecplot(string); //OK
    bool selected; //OK
};

extern vector<CPARDomain*>dom_vector;
extern void DOMRead(string);
extern void DOMCreate();
/*---- MPI Parallel --------------*/
extern int size;
extern int myrank;
extern char t_fname[3];
extern double time_ele_paral;
/*---- MPI Parallel --------------*/

#define DDC_FILE_EXTENSION ".ddc"
void DOMWriteTecplot(string);

#endif
