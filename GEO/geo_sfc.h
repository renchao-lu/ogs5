#ifndef rf_sfc_INC

#define rf_sfc_INC

#include "geo_ply.h"

//-------------------------------------------------------------------------
class CTriangle
{
  public:
    long number;
    double x[3];
    double y[3];
    double z[3];
    long msh_numbers[3];
};
//-------------------------------------------------------------------------
class CTIN
{
  public:
     CTIN(){};
    ~CTIN();
    string name;
    vector<CTriangle*> Triangles;
};

//-------------------------------------------------------------------------
class Surface{
public:
	Surface();
	~Surface();
	//ID
    long id;//CC
    string name;
    //Properties
	int type;
    string type_name;
    //int data_type;
    double epsilon;
    int mat_group; // MMP
    string mat_group_name;
    double Radius; // Radius of cylinder. WW
    //display
    int m_color[3];
    int display_mode_2d;
    int display_mode_3d;
    int display_mode_bc;
    bool highlighted;// CC
    //topology
    bool order; 
    bool createtins;
    double center_point[3];
    //TIN
    CTIN *TIN;
    //point vector
    vector<CGLPoint*>polygon_point_vector;
    // polylines
   // list<CGLPolyline*> polyline_of_surface_list;
    vector<CGLPolyline*> polyline_of_surface_vector;
    vector<int> polyline_of_surface_orient;
    vector<double*>nodes_coor_vector;  
    // MSH
    int meshing_allowed; //TK
    //----------------------------------------------------------------
    //Method
    // I/O
	void output(FILE* geo_file, int &p_index, int &l_index, 
		                        int &pl_index,int &s_index);
    void Write(string);
    ios::pos_type Read(ifstream*,string);
    //Topology
    void PolylineOrientation();//CC
    void ReArrangePolylineList();
    void PolygonPointVector();//OK/CC
    //point
    void CalcCenterPoint(void);
    //display
    void AssignColor();//CC
    // TIN
    void CreateTIN(void);
    void ReadTIN(string);//CC
    void WriteTIN(string);//CC
    void WriteTINTecplot(string);//CC
    //Tecplot
    void WriteTecplot(fstream*);
    bool PointInSurface(CGLPoint*); //OK
  private:
    friend class CGLLine; //WW
   
}; 
//vector
typedef vector<Surface*> surface_vec; //CC
extern vector<Surface*> surface_vector;//CC
extern vector<Surface*> GetSurfaceVector(void);//CC
extern void GEOCreateSurfacePointVector(void);//CC
//Access
extern Surface* GEOGetSFCByName(string);
//I/O
extern void GEOReadSurfaces(string file_name_path_base);
extern void GEOWriteSurfaces(string);//C
//Remove
extern void GEORemoveAllSurfaces();//CC
extern void GEORemoveSurface(surface_vec::iterator Iter);//CC
extern void GEORemoveSFC(Surface*m_sfc);
//Topology
extern void GEOSurfaceTopology(void);
extern void GEOUnselectSFC(); //OK
//TIN
#define TIN_FILE_EXTENSION ".tin"
extern void GEOWriteSurfaceTINs(string);//TIN
extern void GEOCreateLayerSurfaceTINs(int,double);//TIN
extern void GEOWriteSurfaceTINsTecplot(string);
extern int sfc_ID_max;
// MSH
void MSHUnselectSFC(); //OK
extern int SFCGetMaxMATGroupNumber(); //OK
extern void SFCAssignMATColors(); //OK
extern void SFCAssignColors(); //OK

#endif
