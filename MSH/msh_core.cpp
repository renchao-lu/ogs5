/**************************************************************************
MSHLib - Object:
Task:
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#include "msh_core.h"

namespace Mesh_Group
{

/**************************************************************************
MSHLib-Method:
Task:
Programing:
06/2005 WW Implementation
03/2010 TF initialisation in initialisation list
**************************************************************************/
CCore::CCore(size_t id) :
	index (id), boundary_type ('I'), mark (true), quadratic (false)
{}

} // namespace Mesh_Group
