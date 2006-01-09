/**************************************************************************
FEMLib-Object: FCT
Task: Functions
Programing:
01/2005 OK Implementation
**************************************************************************/
#ifndef rf_fct_INC
#define rf_fct_INC
// C++ STL
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

class CFunction
{
  private:
  public:
    CFunction(void);
    ~CFunction(void);
    void Set(string,string,double);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    double GetValue(double,bool*);
  public:
    string type_name;
    string geo_name;
    string geo_type_name;
    vector<string>variable_names_vector;
    vector<double*>variable_data_vector;
    bool selected;
};

extern vector<CFunction*>fct_vector;
extern void FCTWrite(string);
extern void FCTRead(string);
extern void FCTReadTIMData(string);
extern CFunction* FCTGet(string);

#define FCT_FILE_EXTENSION ".fct"

#endif
