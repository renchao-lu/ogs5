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
   vector<VTK_Info> vec_dataset;
   string pvd_file_name;
   string pvd_vtk_file_name_base;

public:
  CVTK(void){};
  virtual ~CVTK(void){};

protected:
  bool WriteHeaderOfPVD(fstream &fin);
  bool WriteEndOfPVD(fstream &fin);
  bool WriteDatasetOfPVD(fstream &fin, double timestep, const string &vtkfile);
public:
  bool InitializePVD(const string &file_base_name, const string &pcs_type_name);
  bool UpdatePVD(const string &pvdfile, const vector<VTK_Info> &vec_vtk);
  bool CreateDirOfPVD(const string &pvdfile);
  bool WriteXMLUnstructuredGrid(const string &vtkfile, COutput *out, const int time_step_number);

public:
  int GetVTKCellType(const int ele_type) ;
};

#endif
