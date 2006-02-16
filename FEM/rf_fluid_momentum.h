////////////////////////////////////////////////////////////////
//   Filename: rf_fluid_momentum.h
//   
//   written by PCH, 05/2005
////////////////////////////////////////////////////////////////


// C++ STL
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include "rf_pcs.h"
#include "rf_vel_new.h"
#include "fem_ele_std.h"
#include "fem_ele.h"

#include "cgs_asm.h"
#include "matrix.h"

using namespace std;

using namespace FiniteElement;


class CFluidMomentum:public CRFProcess
{
public:  
	CFluidMomentum(void);
	~CFluidMomentum(void);
 

	void Create(void);
	virtual double Execute();
	void SolveDarcyVelocityOnNode();

protected:
	FiniteElement::CFiniteElementStd *fem;


private:    
	CRFProcess* m_pcs;
};

extern void FMRead(string);
extern void DATWriteHETFile(const char *file_name);



