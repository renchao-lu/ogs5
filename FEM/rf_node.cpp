/**************************************************************************
FEMLib-Object: CNodeValue
Task: Functions
Programing:
01/2005 OK Implementation
**************************************************************************/

#include "stdafx.h" /* MFC */
#include "rf_node.h"

/**************************************************************************
FEMLib-Method:
Task: constructor
Programing:
04/2005 OK/WW Implementation
last modification:
**************************************************************************/
CNodeValue::CNodeValue()
{
  geo_node_number = -1;
  msh_node_number = -1;
  node_value = 0.;
  node_area = 0.;
  node_distype = -1;
  node_parameterA = 0.;
  node_parameterB = 0.;
  node_parameterC = 0.;
  node_parameterD = 0.;
  node_parameterE = 0.;
  CurveIndex = -1;
  conditional = -1;
 
}

/**************************************************************************
FEMLib-Method:
Task: destructor
Programing:
04/2005 OK/WW Implementation
last modification:
**************************************************************************/
CNodeValue::~CNodeValue()
{
}
