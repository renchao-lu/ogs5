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
/**************************************************************************
FEMLib-Method:
Task: destructor
Programing:
03/2006 WW Implementation
last modification:
**************************************************************************/
void CNodeValue::Write(ostream& os) const
{
   string deli = "  ";
   os<<geo_node_number<<deli;
   os<<msh_node_number<<deli;
   os<< CurveIndex <<deli;
   os<< node_value <<deli;
   /*
   // This is for river flow
   // This writing will be valid for river flow when some 
   // of its parameters being moved from CSourceTerm to here
   os<< node_distype <<deli;
   os<< node_area <<deli;
   os<< node_parameterA <<deli;
   os<< node_parameterB <<deli;
   os<< node_parameterC <<deli;
   os<< node_parameterD <<deli;
   os<< node_parameterE <<deli;
   os<< conditional <<deli;
   */
   os<<endl;  
}

/**************************************************************************
FEMLib-Method:
Task: destructor
Programing:
03/2006 WW Implementation
last modification:
**************************************************************************/
void CNodeValue::Read(istream& is) 
{
   is>>geo_node_number;
   is>>msh_node_number;
   is>> CurveIndex ;
   is>> node_value ;
   /* 
   // This is for river flow
   // This writing will be valid for river flow when some 
   // of its parameters being moved from CSourceTerm to here
   is>> node_distype ;
   is>> node_area ;
   is>> node_parameterA ;
   is>> node_parameterB ;
   is>> node_parameterC ;
   is>> node_parameterD ;
   is>> node_parameterE ;
   is>> conditional ;
   */
   is>> ws;  
}
