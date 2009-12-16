/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#include "stdafx.h" // MFC
#include "msh_core.h"

namespace Mesh_Group
{

/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
CCore::CCore(const long id)
{
  index = id;
  mark = true;
  quadratic = false;
  boundary_type = 'I';
}

} // namespace Mesh_Group
