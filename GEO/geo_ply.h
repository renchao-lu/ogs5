/**************************************************************************
GeoLib - Object: Polyline
Task: 
Programing:
07/2003 OK/CC/TK/WW GEOLib1
07/2005 CC/OK  GEOLib2 Design
**************************************************************************/
#ifndef gl_ply_INC
#define gl_ply_INC
#include "geo_pnt.h" 
#include "geo_lin.h" 
#define PLY_FILE_EXTENSION ".ply"
enum PLY_TYPES {GEO,MSH,IC,BC,ST};

class CGLPolyline
{
  private:
    long number;
	friend class Surface;
  public:
    // constructor
    CGLPolyline(void);
    CGLPolyline(string); //OK
    // destructor
    ~CGLPolyline(void);
    // ID
    long id;//CC
    string name;
    string ply_name;
    //properties
    int type;
    int data_type;
    string ply_type;//CC9999
    string ply_data;//CC9999
    double epsilon;
    int mat_group;
    double minDis;
    double mesh_density;//CC9999 ply density
    //display
    bool highlighted;// CC
    int m_color[3];
    int display_mode;
    bool closed;
    double min_plg_Dis;
    //components
	bool computeline;
    int number_of_points;
    string ply_file_name;
    vector<CGLPoint*> point_vector;
	vector<CGLLine*> line_vector;
    vector<double> sbuffer;
    vector<int> ibuffer;
	//I/O
    ios::pos_type Read(ifstream*,string);//CC
    void Write(char* file_name);
    //display
    void AssignColor();//CC
    //method
    void ComputeLines(CGLPolyline*);
    bool PointExists(CGLPoint* point,CGLPoint* point1);
    void AddPoint(CGLPoint* m_point);
    CGLPoint* CenterPoint(void);
	//point vector
    void WritePointVector(string);//CC
    void ReadPointVector(string);//CC
    void SortPointVectorByDistance();
    //write tecplot file
    void WriteTecplot(string);//CC
    // Meshing
    vector<long>msh_nodes_vector;
    // vector<double*> msh_coor_vector; //HS: never used. removed 30.10.2009
    vector<int> OrderedPoint;
    void GetPointOrderByDistance();  
    void SetPointOrderByDistance(CGLPoint*); //OK
    void CalcMinimumPointDistance(); //OK
	#ifdef RFW_FRACTURE
	double CalcPolylineLength(); //RFW
	#endif
};

extern vector<CGLPolyline*> polyline_vector;//CC
extern vector<CGLPolyline*> GetPolylineVector(void);//CC
//Access
extern CGLPolyline* GEOGetPLYByName(string);
extern CGLPolyline* GEOGetPLYById(long);// CC
//methods
extern void GEOPolylineGLI2GEO(FILE *geo_file);
extern void GEOUnselectPLY(); //OK
//Remove
extern void GEORemoveAllPolylines();//CC
extern void GEORemovePolyline(long);//CC 03/06
extern void GEORemovePLY(CGLPolyline*); //OK
//I/O
extern void GEOReadPolylines(string file_name_path_base);
extern void GEOWritePolylines(char* file_name);//CC
//RF
extern void InterpolationAlongPolyline(CGLPolyline *plyL, vector<double>& bcNodalValue);  

class CColumn : public CGLPolyline //OK
{
  private:
    CGLLine* m_lin;
  public:
    ~CColumn();
    double geo_area;
    double center_point[3];
};


class CSoilProfile : public CGLPolyline  //YD
{
public:
   CSoilProfile();
    ~CSoilProfile();
   long profile_type;
   string soil_name;
   vector<long>soil_type;
   vector<double>soil_layer_thickness;
};

extern void COLDeleteLines();
extern void COLDelete();
extern CColumn* COLGet(int);
extern CColumn* COLGet(string);

extern vector<CColumn*>column_vector;
extern vector<CSoilProfile*>profile_vector;  //YD
#endif
