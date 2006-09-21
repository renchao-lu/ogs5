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

namespace FiniteElement { class CFiniteElementVec;}
using FiniteElement::CFiniteElementVec;
namespace process { class CRFProcessDeformation;}
using process::CRFProcessDeformation;


#if defined(USE_MPI) 
//WW
namespace Mesh_Group {class CFEMesh;}
using  Mesh_Group::CFEMesh;
#endif
void FindNodesOnInterface( CFEMesh *m_msh, bool quadr);

//-----------------------------------------------
class CPARDomain
{
  private:
    vector<long*> element_nodes_dom; //WW
    vector<long*> node_conneted_nodes; //WW
    vector<int> num_nodes2_node; //WW
    long nnodes_dom;
    long nnodesHQ_dom;
//#ifdef USE_MPI //WW
    long num_inner_nodes;
    long num_inner_nodesHQ;
    long num_boundary_nodes;
    long num_boundary_nodesHQ;
    friend void FindNodesOnInterface(CFEMesh *m_msh, bool quadr);
//#endif
    long shift[4]; //WW
    friend class CRFProcess; //WW
    friend class FiniteElement::CFiniteElementVec; //WW //process:: for SXC compiler
    friend class process::CRFProcessDeformation; //WW //process:: for SXC compiler

  public:
    int ID;
    vector<long> elements;
    vector<long> nodes_inner;
    vector<long> nodes_halo;
    vector<long> nodes;
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
    void CreateElements(const bool quadr); // const bool quadr. WW
    void NodeConnectedNodes(); //WW
    //
    void CreateEQS(CRFProcess *m_pcs);
    void CalcElementMatrices(CRFProcess*);
    void AssembleMatrix(CRFProcess*);
    void WriteMatrix();
    void SolveEQS();
    long GetDOMNode(long);
    int m_color[3]; //OK
    void WriteTecplot(string); //OK
    
    bool selected; //OK
    bool quadratic; //WW
    long GetDomainNodes() const  //WW
        {if(quadratic) return nnodesHQ_dom;
	  else  return  nnodes_dom;  }
#if defined(USE_MPI) //WW
    long GetNumInnerNodes(bool quadr)
       {if(quadr) return num_inner_nodesHQ;
       else  return num_inner_nodes; }   
    long GetNumHaloNodes(bool quadr)
       {  if(quadr) return num_boundary_nodesHQ;
          else  return num_boundary_nodes;}
#endif
};

extern vector<CPARDomain*> dom_vector;

extern vector<double> node_connected_doms; // WW

extern void DOMRead(string);
extern void DOMCreate( CRFProcess *m_pcs);
//---- MPI Parallel --------------
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL) //WW
extern int size;
extern int myrank;
extern char t_fname[3];
extern double time_ele_paral;
#endif
//---- MPI Parallel --------------


#define DDC_FILE_EXTENSION ".ddc"
void DOMWriteTecplot(string);

#endif
