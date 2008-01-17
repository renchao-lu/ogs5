/**************************************************************************
GeoLib - Object: Point
Task: 
Programing:
07/2003 OK/CC/TK/WW GEOLib1
07/2005 CC/OK GEOLib2 Design
**************************************************************************/
#ifndef gs_pnt_INC
#define gs_pnt_INC
// C++ STL
#include <string>
#include <vector>
#include "geo_mathlib.h"
//
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;
/*---------------------------------------------------------------*/
class CGLPoint
{
  private:

  public:
  //----------------------------------------------------------------------
  // Properties 
    // ID
   string name;
  //  char* name;
    long id; //CC
    // Geometry
    double x;
    double y;
    double z;
    double epsilon;
    int mat;//CC9999
    double length; //OK well bore depth in 2D modells
	long first_identical_id; //TK
	long old_id, new_id; //TK
    int nb_of_ply; //TK Number of Polylines using this point
	long number_of_doubled_points; //TK
    // Meshing
    int index_msh; //WW
    double min_seg_length;//TK for polylines
    double max_seg_length;//TK
    double mesh_density;
    // Properties
    double value;
    double propert;
    // Display
    bool highlighted;
    int x_pix, y_pix;
    int circle_pix;
    int display_mode;
	int m_color[3]; 
    bool selected;
    int plg_hightlight_seg;
  //----------------------------------------------------------------------
  // Methods
    // Create
	CGLPoint(void);
    ~CGLPoint(void);
  
    // Access
	void SetIndex(const int L_index) {index_msh=L_index;}
	int  GetIndex() {return index_msh;}
    // I/O
   // void Write(char*);
    ios::pos_type Read(ifstream*,int*);//CC
    // GEO
    int IsPointExist();//CC
    CGLPoint* Exist(); //OK
    double PointDis(CGLPoint*); //CC
    double PointDisXY(CGLPoint*); //OK
    bool IsInsidePolygonPlain(double*,double*,double*,long); //CC IsPointInsideSurface()?
    bool IsInsideTriangle(double*,double*,double*);//CC
    bool IsInTriangleXYProjection(double*,double*);//CC
    bool IsInsideRectangle(double*,double*,double*);//CC
    bool IsInsidePrism(double*,double*,double*);//CC
};
//------------------------------------------------------------------------
// Properties
extern vector<CGLPoint*> GetPointsVector(void);//CC
extern vector<CGLPoint*> gli_points_vector;
extern vector<CGLPoint*> pnt_properties_vector; //OK

//------------------------------------------------------------------------
// Remove
extern void GEORemoveAllPoints();
extern void GEORemovePoint(long);//CC
//........................................................................
// I/O
extern void GEOReadPoints(string file_name_path_base);//CC
extern void GEOReadPointProperties(string);
extern void GEOWritePoints(char* file_name);//CC
//........................................................................
// Access
extern CGLPoint* GEOGetPointByName(string);//CC 
extern CGLPoint* GEOGetPointById(long);//CC
//........................................................................
// GEO
extern long GEOPointID();
extern void GEO_Search_DoublePoints(double);//TK
extern vector<CGLPoint*> GEOLIB_SetGLIPoints_Vector(vector<CGLPoint*> gl_point); //TK
extern double AngleSumPointInsideTriangle(double *point, double *tri_p1,double *tri_p2,double *tri_p3, double tolerance);
extern void GEOCalcPointMinMaxCoordinates(); //OK
extern int GEOMaxPointID(); //OK
//........................................................................
//variables
extern double pnt_x_min;
extern double pnt_x_max;
extern double pnt_y_min;
extern double pnt_y_max;
extern double pnt_z_min;
extern double pnt_z_max;
//Id
extern long pnt_id_last;
#endif
