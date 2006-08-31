/**************************************************************************
FEMLib - Object: Node
Task: class implementation
Programing:
04/2004 OK Implementation
last modified
02/2005 MB node parameter....
**************************************************************************/
#ifndef rf_node_INC

#define rf_node_INC

// C++ STL
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

//========================================================================
class CNodeValue
{
  public:
	CNodeValue(void);
    ~CNodeValue(void);
 //
    long geo_node_number;
    long msh_node_number;
    double node_value;
    double node_area;
    int node_distype;
    double node_parameterA;
    double node_parameterB;
    double node_parameterC;
    double node_parameterD;
    double node_parameterE;
    int CurveIndex;
    int conditional;
    vector<double>history_value;
    long msh_node_number_conditional;
    string tim_type_name;
    void Write(ostream& os=cout) const; //WW
    void Read(istream& is=cin); //WW
    bool check_me; //OK
};

//========================================================================
/* // Commented by WW
class CNodeValueGroup
{
  private:
  public:
    string pcs_name;
    vector<CNodeValue*>group_vector;
	//CNodeValueGroup(void);
    //~CNodeValueGroup(void);
    void Set(string);
    CNodeValueGroup* Get(string);
};
*/
#endif
