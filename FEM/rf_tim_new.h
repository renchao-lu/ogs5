/**************************************************************************
FEMLib - Object: TIM
Task: class implementation
Programing:
06/2004 OK Implementation
last modified:
**************************************************************************/
#ifndef rf_tim_new_INC
#define rf_tim_new_INC
// C++ STL
#include <fstream>
#include <string>
#include <vector>
using namespace std;
// GEOLib
// FEMLib
//----------------------------------------------------------------
class CTimeDiscretization
{
  private:
  public:
    string file_base_name;
    // TIM
    vector<double>time_step_vector;
	vector<int> time_adapt_tim_vector;
    vector<double>time_adapt_coe_vector;
    double time_start;
    double time_end;
    double time_current;
	double time_control_manipulate; //CMCD
    int step_current;
    // PCS
    string pcs_type_name; //OK
    // NUM
    string time_type_name; //OK
    string time_control_name;
    string time_unit;  //WW
	double iter_times;  //YD
	double multiply_coef; //YD
	bool Write_tim_discrete;  //YD
	fstream *tim_discrete;  //YD
  public:
    CTimeDiscretization(void);
    ~CTimeDiscretization(void);
    ios::pos_type Read(ifstream*);
    void Write(fstream*);
    double time_step_length_neumann; //YD
    double time_step_length; //YD
    double CalcTimeStep();
    double FirstTimeStepEstimate();
    double NeumannTimeControl();
    double SelfAdaptiveTimeControl();
};

extern vector<CTimeDiscretization*>time_vector;
extern bool TIMRead(string);
extern CTimeDiscretization* TIMGet(string);
extern void TIMWrite(string);
extern void TIMDelete();
extern void TIMDelete(string);
#define TIM_FILE_EXTENSION ".tim"
//ToDo
extern double aktuelle_zeit;
extern long aktueller_zeitschritt;
extern double dt;
#endif
