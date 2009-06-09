#include "vtk.h"

#include "stdafx.h" /* MFC */
#include <fstream>
#if defined(WIN32)
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "makros.h"
#include "rf_mmp_new.h"
#include "fem_ele_std.h" // for element velocity

const string INDEX_STR = "  ";

bool CVTK::WriteHeaderOfPVD(const string &pvdfile)
{
  fstream fin(pvdfile.data(), ios::out);
  if (!fin.good()) return false;
  fin << "<?xml version=\"1.0\"?>" << endl;
  fin << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">"  << endl;
  fin << INDEX_STR << "<Collection>" << endl;
  fin.close();
  return true;
}

bool CVTK::WriteEndOfPVD(const string &pvdfile)
{
  fstream fin(pvdfile.data(), ios::app);
  if (!fin.good()) return false;
  fin << INDEX_STR << "</Collection>" << endl;
  fin << "</VTKFile>" << endl;
  fin.close();
  return true;
}

bool CVTK::WriteDatasetOfPVD(const string &pvdfile, double timestep, const string &vtkfile)
{
  fstream fin(pvdfile.data(), ios::app);
  if (!fin.good()) return false;
  fin.setf(ios::scientific,ios::floatfield);
  fin.precision(12);
  fin << INDEX_STR << INDEX_STR << "<DataSet timestep=\"" << timestep << "\" group=\"\" part=\"0\" file=\"" << vtkfile << "\"/>" << endl;
  fin.close();
  return true;
}

bool CVTK::UpdatePVD(const string &pvdfile, const vector<VTK_Info> &vec_vtk)
{
  CVTK::WriteHeaderOfPVD(pvdfile);
  for (int i=0; i<(int)vec_vtk.size(); i++)
    CVTK::WriteDatasetOfPVD(pvdfile, vec_vtk[i].timestep, vec_vtk[i].vtk_file);
  CVTK::WriteEndOfPVD(pvdfile);

  return true;
}

bool CVTK::CreateDirOfPVD(const string &pvdfile)
{
  string pvd_dir_path = pvdfile + ".d";
#if defined(WIN32)
  if (mkdir(pvd_dir_path.c_str()) == -1) {
#else
  if (mkdir(pvd_dir_path.c_str(), 0777) == -1) {
#endif
    //error
    cout << "ERROR: Fail to create a PVD directory: " << pvd_dir_path << endl;
    return false;
  }
  return true;
}

int CVTK::GetVTKCellType(const int ele_type) 
{
  int cell_type = -1;

  switch(ele_type){
    case 1:  // vtk_line=3
        cell_type = 3;
        break;
    case 2:  // quadrilateral=9
        cell_type = 9;
        break;
    case 3: // hexahedron=12
        cell_type = 12;
        break;
    case 4:  // triangle=5
        cell_type = 5;
        break;
    case 5:  // tetrahedron=10
        cell_type = 10;
        break;
    case 6:   // wedge=13
        cell_type = 13;
      break;
    default:
      cerr << "ERROR ** NO CORRESPONDING VTK CELL TYPE FOUND. (ELEMENT TYPE=" << ele_type << ")" << endl;
  }
  return cell_type;
}

bool CVTK::WriteXMLUnstructuredGrid(const string &vtkfile, COutput *out, const int time_step_number)
{
  fstream fin(vtkfile.data(), ios::out);
  if (!fin.good()) return false;
  fin.setf(ios::scientific,ios::floatfield);
  fin.precision(12);

  CFEMesh *msh = out->GetMSH();

  fin << "<?xml version=\"1.0\"?>" << endl;
  fin << "<!-- Time step: " << time_step_number << " | Time: " << out->time << " -->" << endl;
  fin << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">"  << endl;
  fin << "  <UnstructuredGrid>" << endl;
  fin << "    <Piece NumberOfPoints=\"" << msh->GetNodesNumber(false) << "\" NumberOfCells=\"" << msh->ele_vector.size() << "\">" << endl;
  //....................................................................
  // Nodes
  CNode *nod = NULL;
  fin << "      <Points>" << endl;
  fin << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
  for (long i=0; i<(long)msh->nod_vector.size(); i++) {
    nod = msh->nod_vector[i];
    fin << "          " << nod->X() << " " << nod->Y() << " " << nod->Z() << endl;
  }
  fin << "        </DataArray>" << endl;
  fin << "      </Points>" << endl;
  //....................................................................
  // Elements
  CElem * ele = NULL;
  fin << "      <Cells>" << endl;
  fin << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << endl;
  for (long i=0; i<(long)msh->ele_vector.size(); i++) {
    ele = msh->ele_vector[i];
    fin << "          ";
    for (long j=0; j<msh->ele_vector[i]->GetNodesNumber(false); j++)
      fin << ele->GetNodeIndex(j) << " ";
    fin << endl;
  }
  fin << "        </DataArray>" << endl;
  fin << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << endl;
  fin << "          ";
  long offset = 0;
  for (long i=0; i<(long)msh->ele_vector.size(); i++) {
    ele = msh->ele_vector[i];
    offset += msh->ele_vector[i]->GetNodesNumber(false);
    fin << offset << " ";
  }
  fin << endl;
  fin << "        </DataArray>" << endl;
  fin << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << endl;
  fin << "          ";
  for(long i=0;i<(long)msh->ele_vector.size();i++){
    ele = msh->ele_vector[i];
    fin << this->GetVTKCellType(ele->GetElementType()) << " ";
  }
  fin << endl;
  fin << "        </DataArray>" << endl;
  fin << "      </Cells>" << endl;
  //....................................................................
  // Nodal values
  if (out->nod_value_vector.size() > 0) {
    fin << "      <PointData Scalars=\"" << out->nod_value_vector[0] << "\">" << endl;
  } else {
    fin << "      <PointData Scalars=\"scalars\">" << endl;
  }
  CRFProcess* m_pcs = NULL;
  vector<int> NodeIndex(out->nod_value_vector.size());
  bool outNodeVelocity = false;
  for (int i=0; i<(int)out->nod_value_vector.size(); i++) {
    if (out->nod_value_vector[i].find("VELOCITY")!=string::npos) {
      outNodeVelocity = true;
      continue;
    }
    m_pcs = PCSGet(out->nod_value_vector[i],true);
    if(!m_pcs) continue;
    NodeIndex[i] = m_pcs->GetNodeValueIndex(out->nod_value_vector[i]);
    if(NodeIndex[i]<0) continue;

    for(int j=0; j<m_pcs->GetPrimaryVNumber(); j++)
    {
      if(out->nod_value_vector[i].compare(m_pcs->pcs_primary_function_name[j])==0)
      {
         NodeIndex[i]++;
         break;
       }
    }
    fin << "        <DataArray type=\"Float64\" Name=\""<< out->nod_value_vector[i] << "\" format=\"ascii\">" << endl;
    fin << "          ";
    for(int j=0; j<msh->GetNodesNumber(false); j++) {
      fin << m_pcs->GetNodeValue(msh->nod_vector[j]->GetIndex(), NodeIndex[i]) << " ";
    }
    fin << endl;
    fin << "        </DataArray>" << endl;
  }
  // Nodal velocities 
  if (outNodeVelocity) {
    const static string velocity_name[][4] = {{"VELOCITY_X1", "VELOCITY_Y1", "VELOCITY_Z1", "NODAL_VELOCITY1"}, {"VELOCITY_X2", "VELOCITY_Y2", "VELOCITY_Z2", "NODAL_VELOCITY2"}};
    unsigned int velocity_id = 0;
    for (int i=0; i<(int)out->nod_value_vector.size(); i++) {
      m_pcs = NULL;
      if (out->nod_value_vector[i].find("VELOCITY_X1")!= string::npos) {
        m_pcs = PCSGet(out->nod_value_vector[i],true);
        velocity_id = 0;
      } else if (out->nod_value_vector[i].find("VELOCITY_X2")!= string::npos) {
        m_pcs = PCSGet(out->nod_value_vector[i],true);
        velocity_id = 1;
      }
      if(!m_pcs) continue;

      int ix,iy,iz;
      ix = m_pcs->GetNodeValueIndex(velocity_name[velocity_id][0]);
      iy = m_pcs->GetNodeValueIndex(velocity_name[velocity_id][1]);
      iz = m_pcs->GetNodeValueIndex(velocity_name[velocity_id][2]);
      fin << "        <DataArray type=\"Float64\" Name=\""<< velocity_name[velocity_id][3] << "\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
      fin << "          ";
      for(int j=0l;j<msh->GetNodesNumber(false);j++)
      {
         fin << m_pcs->GetNodeValue( msh->nod_vector[j]->GetIndex(),ix) << " ";
         fin << m_pcs->GetNodeValue( msh->nod_vector[j]->GetIndex(),iy) << " ";
         fin << m_pcs->GetNodeValue( msh->nod_vector[j]->GetIndex(),iz) << " ";
//         fin << endl;
      }
      fin << endl;
      fin << "        </DataArray>" << endl;
    }
  }
  fin << "      </PointData>" << endl;

  //======================================================================
  //....................................................................
  // Element values
  fin << "      <CellData>" << endl;
  vector<int>ele_value_index_vector(out->ele_value_vector.size());
  if (ele_value_index_vector.size() > 0) // GetELEValuesIndexVector() should check this!
    out->GetELEValuesIndexVector(ele_value_index_vector);
  bool outEleVelocity = false;
  for(int i=0;i<(int)ele_value_index_vector.size();i++)
  {
    if (out->ele_value_vector[i].find("VELOCITY")!=string::npos) {
      outEleVelocity = true;
      continue;
    }
    m_pcs = out->GetPCS_ELE(out->ele_value_vector[i]);
    fin << "        <DataArray type=\"Float64\" Name=\""<< out->ele_value_vector[i] << "\" format=\"ascii\">" << endl;
    fin << "          ";
    for(long j=0;j<(long)msh->ele_vector.size();j++) {
	  fin << m_pcs->GetElementValue(j,ele_value_index_vector[i]) << " ";
    }
    fin << endl;
    fin << "        </DataArray>" << endl;
  }
  if (outEleVelocity) {
    fin << "        <DataArray type=\"Float64\" Name=\"ELEMENT_VELOCITY\" NumberOfComponents=\"3\" format=\"ascii\">" << endl;
    fin << "          ";
    static double ele_vel[3]={0.0,0.0,0.0};
    for(long i=0;i<(long)msh->ele_vector.size();i++)
    {
      ele_gp_value[i]->getIPvalue_vec(0, ele_vel);
      fin << ele_vel[0] << " ";
      fin << ele_vel[1] << " ";
      fin << ele_vel[2] << " ";
//      fin << endl;
    }
    fin << endl;
    fin << "        </DataArray>" << endl;
  }
  if(mmp_vector.size() > 1)
  {
    fin << "        <DataArray type=\"Int32\" Name=\"MatGroup\" format=\"ascii\">" << endl;
    fin << "          ";
    for(long i=0;i<(long)msh->ele_vector.size();i++)
    {
      ele = msh->ele_vector[i];
      fin << ele->GetPatchIndex() << " ";
    }
    fin << endl;
    fin << "        </DataArray>" << endl;
  }
  fin << "      </CellData>" << endl;
  fin << "    </Piece>" << endl;
  fin << "  </UnstructuredGrid>" << endl;
  fin << "</VTKFile>" << endl;
  fin.close();

  return true;
}

