/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulated from mshlib
**************************************************************************/

#include <iomanip>
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_node.h"
// PCSLib
#ifdef RFW_FRACTURE
#include "rf_pcs.h" //RFW 06/2006
#endif
//========================================================================
namespace Mesh_Group
{
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
CNode::CNode(const int Index, const double x, 
            const double y, const double z):CCore(Index)
{
  
   eqs_index = -1; 
   coordinate[0] = x;
   coordinate[1] =y;
   coordinate[2] =z; 
   free_surface=-1; 
   interior_test=0;
   selected=0;
   epsilon=0.0;
   // PCH
   crossroad = 0;	// Set to be no crossroad
   patch_area = -1.; //OK4310
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CNode::operator = (const CNode& n)
{
   boundary_type = n.boundary_type; 
   index = n.index;
   mark = n.mark;
   eqs_index = n.eqs_index;
   coordinate[0] = n.coordinate[0]; 
   coordinate[1] = n.coordinate[1]; 
   coordinate[2] = n.coordinate[2]; 
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
bool CNode::operator == (const CNode& n)
{
   if(index == n.index)
      return true;
   else 
	  return false;
}
/**************************************************************************
MSHLib-Method: 
06/2005 WW Implementation
03/2006 OK patch_area
**************************************************************************/
void CNode::Write(ostream& osm) const
{
  osm.setf(ios::scientific, ios::floatfield);
  string deli(" ");
  setw(14);
  osm.precision(14);
  osm<<index<<deli
     <<coordinate[0]<<deli
     <<coordinate[1]<<deli
     <<coordinate[2]<<deli; 
  if(patch_area>0.0)
  {
    osm<<"$AREA"<<deli<<patch_area;
  }
  osm<<endl;
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CNode::SetCoordinates(const double* argCoord)
{
    coordinate[0] = argCoord[0];
  	coordinate[1] = argCoord[1];
  	coordinate[2] = argCoord[2];
}

#ifdef RFW_FRACTURE
/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::X_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double x_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_X1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_X1")+1;
	}
	x_displaced = X() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return x_displaced;
}

/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::Y_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double y_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_Y1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_Y1")+1; 
	}
	y_displaced = Y() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return y_displaced;
}

/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::Z_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double z_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_Z1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_Z1")+1;
	}
	z_displaced = Z() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return z_displaced;
}
#endif

} // namespace Mesh_Group
//========================================================================

