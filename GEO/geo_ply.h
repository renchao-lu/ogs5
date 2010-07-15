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
#include <fstream>
#include <string>
#define PLY_FILE_EXTENSION ".ply"
enum PLY_TYPES {GEO,MSH,IC,BC,ST};

class CGLPolyline {
private:
	long number;
	friend class Surface;
public:
	// constructor
	CGLPolyline(void);
	CGLPolyline(const std::string &); //OK
	// destructor
	~CGLPolyline(void);
	// ID
	long id;//CC
	std::string name;
	std::string ply_name;
	//properties
	int type;
	int data_type;
	std::string ply_type;//CC9999
	std::string ply_data;//CC9999
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
	std::string ply_file_name;
	std::vector<CGLPoint*> point_vector;
	std::vector<CGLLine*> line_vector;
	std::vector<double> sbuffer;
	std::vector<int> ibuffer;
	//I/O
	std::ios::pos_type Read(std::ifstream&); // TF , const std::string &);//CC
    void Write(char* file_name);
    //display
    void AssignColor();//CC
    //method
    void ComputeLines(CGLPolyline*);
    bool PointExists(CGLPoint* point,CGLPoint* point1);
    void AddPoint(CGLPoint* m_point);
    CGLPoint* CenterPoint(void);
	//point vector
	void WritePointVector(const std::string &);//CC
	void ReadPointVector(const std::string &);//CC
	void SortPointVectorByDistance();
	//write tecplot file
	void WriteTecplot(const std::string &);//CC
	// Meshing
	std::vector<long> msh_nodes_vector;
	std::vector<double*> msh_coor_vector;
	std::vector<int> OrderedPoint;
	void GetPointOrderByDistance();
	void SetPointOrderByDistance(CGLPoint*); //OK
	void CalcMinimumPointDistance(); //OK
#ifdef RFW_FRACTURE
	double CalcPolylineLength(); //RFW
#endif
};

extern std::vector<CGLPolyline*> polyline_vector;//CC
extern std::vector<CGLPolyline*> GetPolylineVector(void);//CC
//Access
extern CGLPolyline* GEOGetPLYByName(std::string);
extern CGLPolyline* GEOGetPLYById(long);// CC
//methods
extern void GEOPolylineGLI2GEO(FILE *geo_file);
extern void GEOUnselectPLY(); //OK
//Remove
extern void GEORemoveAllPolylines();//CC
extern void GEORemovePolyline(long);//CC 03/06
extern void GEORemovePLY(CGLPolyline*); //OK
//I/O
extern void GEOReadPolylines(const std::string &file_name_path_base);
extern void GEOWritePolylines(char* file_name);//CC
//RF
extern void InterpolationAlongPolyline(CGLPolyline *plyL, std::vector<double>& bcNodalValue);

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
   std::string soil_name;
   std::vector<long>soil_type;
   std::vector<double>soil_layer_thickness;
};

extern void COLDeleteLines();
extern void COLDelete();
extern CColumn* COLGet(int);
extern CColumn* COLGet(const std::string &);

extern std::vector<CColumn*>column_vector;
extern std::vector<CSoilProfile*>profile_vector;  //YD
#endif
