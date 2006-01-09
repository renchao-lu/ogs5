 /**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulation from rf_ele_msh
last modified
**************************************************************************/
#ifndef msh_lib_INC
#define msh_lib_INC
// MSHLib
#include "msh_mesh.h"
#include "moveGEOtoMSH.h"//CC
using Mesh_Group::CFEMesh;
using Mesh_Group::CElem;
using Mesh_Group::CNode;
extern vector<Mesh_Group::CFEMesh*>fem_msh_vector;
extern CFEMesh* FEMGet(string msh_name);
extern void MSHCreateNOD2ELERelations(Mesh_Group::CFEMesh*); //OK
extern bool FEMRead(string);
extern void FEMWrite(string);
extern void FEMDeleteAll();
extern void MSHTopology(); //OK
extern void MSHCalcMinMaxMidCoordinates(); //OK
extern double msh_x_min,msh_x_max; //OK
extern double msh_y_min,msh_y_max; //OK
extern double msh_z_min,msh_z_max; //OK
extern double msh_x_mid,msh_y_mid,msh_z_mid; //OK
// Might be removed
void Read_RFI(istream& msh_file, CFEMesh* m_msh);
extern void MSHAssignMATGroup2Elements(string);
extern void MSHCreateQuadsFromPLY(CGLPolyline*,int);
extern void MSHCreatePrismsFromTriangles();
extern void MSHCreateNodes();
extern void MSHDeleteDoubleElements(int);
extern long MSHMarkDoubleElementsType(int);
extern void RFIWriteTecplot();
extern void MSHWriteTecplot();
extern void MSHAssignMATGroup2LineElements();
extern void MSHAssignMATGroup2TrisElements(string);
extern void MSHAssignMATGroup2QuadElements();
extern void MSHAssignMATGroup2TetsElements();
extern void MSHAssignMATGroup2PrisElements();
extern void MSHAssignMATGroup2PrisElementsNew();
extern void MSH2MATPris();
extern void MSHAssignMATGroup2HexsElements();
extern void MSHMapping(const char *dateiname, const int NLayers,const int row, const int DataType,CFEMesh*);
extern void PrismRefine(const int NLayers, const int Layer, const int NSubLayers);
extern void MSHDestroy();
extern void MSHDelete(string);
extern void DATWriteRFIFile(const char *file_name);
extern void DATWritePCTFile(const char *file_name);     // PCH
extern void MSHWriteVOL2TEC(string); //OK
extern bool msh_file_binary; //OK
extern void GMSH2MSH(char*,CFEMesh*);
extern void Mesh_Single_Surface(string surface_name, const char *file_name_const_char);//TK
extern void Select_Nodes_Elements_by_GMSHFile(const char *file_name_const_char);
extern void Clear_Selected_Nodes_Elements();
extern void MSHLayerWriteTecplot(); //OK
extern CFEMesh* MSHGet(string mat_type_name); //OK
extern CFEMesh* MSHGet(string pcs_type_name,string mat_type_name); //OK
extern CFEMesh* MSHGetGEO(string); //OK
extern int MSHSetMaxMMPGroups(); //OK
#endif
