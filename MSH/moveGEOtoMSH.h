#ifndef move_INC
#define move_INC
#include <vector>
using namespace std;
// GEOLib
#include "geo_pnt.h"
#include "geo_lin.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
//MSHLib
namespace Mesh_Group {class CFEMesh;}
using Mesh_Group::CFEMesh;
#include "msh_mesh.h"//CC 10/05
extern void MSHGetNodesClose(vector<long>&, CGLPoint*); //CC
extern long MSHGetNodeNumberClose(double*,CGLPoint*);//CC
extern long* MSHGetNodesClose(long *number_of_nodes, CGLPolyline*);//CC
extern long* MSHGetNodesCloseSorted(long*,CGLPolyline*);//CC
extern void MSHCreateLines(CGLPolyline*);//CC
extern long* GetNodesOnArc(long &NumNodes, CGLPolyline*); 
extern long *GetPointsIn(Surface*,long*);//MSH
extern  vector<long>GetMSHNodesClose(Surface*);//MSH
extern void CreateTINfromMSH(Surface*,long);//MSH
extern void MapTIN(Surface*,string);//MSH
extern void CreateLayerSurfacesTINs(Surface*,long);//MSH
extern void GetMSHNodesCloseAH(Surface*,vector<long>&);//MSH
extern void GetMSHNodesOnSurface(Surface*,vector<long>& NodesS); //WW MSH
extern void GetMSHNodesOnSurfaceXY(Surface*,vector<long>& NodesS); //WW MSH
 extern void GetMSHNodesOnCylindricalSurface(Surface*,vector<long>& NodesS); //WW MSH
extern long* GetPointsInVolume(CGLVolume*,long*); //MB
extern void GEOGetNodesInMaterialDomain(const int MatIndex, vector<long>& Nodes);
extern void GEOGetNodesInMaterialDomain(CFEMesh* m_msh, const int MatIndex,vector<long>& Nodes, bool Order);
extern void SetRFIPointsClose(CGLLine*);//MSH + geomathlib
extern  void CreateMSHLines(CGLLine*);//MSH
extern bool IsPointInSurface(Surface*,CGLPoint*);//CC depend on Geosys.h
extern void MSHCreateLayerPolylines(CGLPolyline*);//CC/OK 08/2005
extern long* MSHGetNodesCloseXY(long *number_of_nodes,CGLPolyline*);//CC/OK
extern long* MSHGetNodesCloseXYSorted(long*,CGLPolyline*); //CC/OK


#endif
