#ifndef VTK_INC

#define VTK_INC

#include <string>
#include "rf_out_new.h"

using namespace std;

typedef struct {
  double timestep; 
  string vtk_file;
} VTK_Info;

class CVTK
{
public:
  CVTK(void){};
  virtual ~CVTK(void){};

public:
  bool WriteHeaderOfPVD(const string &pvdfile);
  bool WriteEndOfPVD(const string &pvdfile);
  bool WriteDatasetOfPVD(const string &pvdfile, double timestep, const string &vtkfile);
  bool UpdatePVD(const string &pvdfile, const vector<VTK_Info> &vec_vtk);
  bool CreateDirOfPVD(const string &pvdfile);
  bool WriteXMLUnstructuredGrid(const string &vtkfile, COutput *out, const int time_step_number);

public:
  int GetVTKCellType(const int ele_type) ;
};

#endif
