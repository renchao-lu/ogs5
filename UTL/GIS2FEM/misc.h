#ifndef MISC_FEMTOOLKITS_H
#define MISC_FEMTOOLKITS_H

#include <string>
#include <vector>

namespace  MeshLib
{
   class CFEMesh;
}
using  MeshLib::CFEMesh;
extern std::vector<MeshLib::CFEMesh*> fem_msh_vector;

extern bool FEMRead(const std::string file_base_name);
void writeOGSMesh(const MeshLib::CFEMesh &mesh, const std::string file_base_name);
#endif
