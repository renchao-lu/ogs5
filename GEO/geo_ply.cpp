#include "stdafx.h" /* MFC */
#ifdef MFC
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#endif
/* Objects */
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_pnt.h"
#include "geo_mathlib.h"
#include "geo_strings.h"
#include "geo_lib.h"
//GSP
#ifdef MFC
#include "gs_project.h"
#endif

CGLPolyline *m_polyline = NULL;
vector<CGLPolyline*> polyline_vector;
int ply_max = -1;
/*----------------------------------------------------------------------*/
// constructor
CGLPolyline::CGLPolyline(void)
{
  name = "POLYLINE";
  ply_name = "POLYLINE";//CC
  closed = false;
  epsilon = 0.01;
  type = 2;
  computeline = false;
  m_color[0] = 0;
  m_color[1] = 0;
  m_color[2] = 0;
  mat_group = -1;
  data_type = 0;
  minDis = 0;
  highlighted = false;  // CC
  ply_max++; //OK
  id = ply_max;
  mesh_density = 100.0;
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
11/2005 OK Implementation
**************************************************************************/
CGLPolyline::CGLPolyline(string ply_name)
{
  name = ply_name;
  closed = false;
  epsilon = 0.01;
  type = 2;
  computeline = false;
  m_color[0] = 0;
  m_color[1] = 0;
  m_color[2] = 0;
  mat_group = -1;
  data_type = 0;
  minDis = 0;
  highlighted = false;  // CC
  ply_max++; //OK
  id = ply_max;
}

// deconstructor
CGLPolyline::~CGLPolyline(void)
{
    sbuffer.clear();
    ibuffer.clear();
    OrderedPoint.clear();
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
08/2005 CC Implementation
**************************************************************************/
CGLPolyline* GEOGetPLYByName(string name)
{
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    m_polyline = *p;
   if(m_polyline->name.compare(name)==0) { 
      return m_polyline;
      break;
    }
    ++p;
  }
  return NULL;
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
08/2005 CC Implementation
**************************************************************************/
CGLPolyline* GEOGetPLYById(long number)
{
  vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
  while(p!=polyline_vector.end()) {
    m_polyline = *p;
   if(m_polyline->id == number) { 
      return m_polyline;
      break;
    }
    ++p;
  }
  return NULL;
}
/**************************************************************************
GeoLib-Method: GEORemovePolylines
Task: 
Programing:
11/2003 CC Implementation
01/2005 OK List destructor
CCToDo Polyline destructor
08/2005 CC Modification
02/2006 CC destructor
05/2006 TK BUGFIX
**************************************************************************/
void GEORemoveAllPolylines()
{
  //CGLPolyline * m_ply = NULL;
  for (int i = 0; i < (int) polyline_vector.size(); i++){
     //m_ply = polyline_vector[0]; //TK: What's that Cui?
     //delete m_ply;
  delete polyline_vector[i];
  polyline_vector[i]=NULL;
  }
  polyline_vector.clear();//CC
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
03/2006 CC destructor
**************************************************************************/
void GEORemovePolyline(long nSel)
{
  CGLPolyline * m_ply = NULL;
  m_ply = polyline_vector[nSel];
  delete m_ply;
  polyline_vector.erase(polyline_vector.begin() + nSel);
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
07/2003 OK Implementation
08/2005 CC Modification
**************************************************************************/
vector<CGLPolyline*> GetPolylineVector(void)
{
  return polyline_vector;
}
/**************************************************************************
GeoLib-Method: GEOPolylineGLI2GEO
Task: convert PLI to GEO data
Programing:
11/2002 WW/OK   Implementation
12/2002 OK Meshing algorithm output
12/2002 OK use specific polylines for meshing (type=1)
01/2002 WW Extend to multi-polyline
09/2003 OK for GeoLib
12/2003 OK construct lines only once
02/2004 WW Surface-wise output
11/2005 TK FOR-LOOP because of GEOLIB2
**************************************************************************/
void GEOPolylineGLI2GEO(FILE *geo_file)
{	
  int i;
  //vector<CGLLine*> gli_lines_vector;
  vector<CGLPoint*> gli_Points_vector;
  long gli_lines_vector_size;
  gli_Points_vector = GetPointsVector();
  /*---------------------------------------------------------------*/
  /*--------------------   Write geo file  ------------------------*/
  /*--------- Mesh density scaling, smoothing method --------------*/
  /*---------------------------------------------------------------*/
  /*---------------------------------------------------------------*/
  /* Meshing algorithm */
  FilePrintString(geo_file, "Mesh.Algorithm = 3;");
  LineFeed (geo_file);
  FilePrintString(geo_file, "Mesh.Smoothing = 4;");
  LineFeed (geo_file);
  //----------------------------------------------------------------------
  // Compute lines
  gli_lines_vector_size = (long)gli_lines_vector.size();
  if(gli_lines_vector_size==0)
  {
     CGLPolyline* p_pline=NULL;
      vector<CGLPolyline*>::iterator p = polyline_vector.begin();//CC
     while(p!=polyline_vector.end()) {
       p_pline = *p;
       if(p_pline->line_vector.size()==0) //CC
         p_pline->ComputeLines(p_pline);
       p++;
	 }
   }
  //----------------------------------------------------------------------
  // Write 
  int p_counter = 0;
  int l_counter = 0;
  int s_counter = 0;
  int pl_counter = 0;
  for (i=0; i<(int)surface_vector.size();i++)
  {
    if(!surface_vector[i]->meshing_allowed){ //OK/TK
      continue; 
    }
    s_counter++;
	surface_vector[i]->output(geo_file, p_counter, l_counter, pl_counter, s_counter);
  }
  //----------------------------------------------------------------------
  // Null the index
  long gli_point_vector_size =(long)gli_Points_vector.size();
  for(i=0;i<gli_point_vector_size;i++)
	  gli_Points_vector[i]->SetIndex(-1);
  for(i=0;i<gli_lines_vector_size;i++)
	  gli_lines_vector[i]->mesh_index = -1;
  //----------------------------------------------------------------------
}
/**************************************************************************
GeoLib-Method: GEOReadPolylineNew
Task: Read polyline data from file
Programing:
07/2003 OK Implementation
12/2003 no line output
06(2005 OK point_vector
11/2005 CC Write function
**************************************************************************/
void CGLPolyline::Write(char* file_name)
{
  FILE *f = NULL;
  const char *filename = 0; 
  string gli_file_name;
  long i;
  //sprintf(gli_file_name,"%s.%s",file_name,"gli");
  gli_file_name = (string)file_name + ".gli";
  filename = gli_file_name.data();
  f = fopen(filename,"a");
  fprintf(f,"#POLYLINE\n");
  fprintf(f," $ID\n");//CC
  fprintf(f,"  %ld\n",id);//CC
  fprintf(f," $NAME\n");
 if (data_type == 1)//CC8888
 fprintf(f,"  %s\n",ply_name.c_str());//CC8888
  else//CC8888
  fprintf(f,"  %s\n",name.c_str());
  fprintf(f," $TYPE\n");
  fprintf(f,"  %d\n",type);
  fprintf(f," $EPSILON\n");
  fprintf(f,"  %g\n",epsilon);
  fprintf(f," $MAT_GROUP\n");
  fprintf(f,"  %d\n",mat_group);
    if(data_type==0) {
      fprintf(f," $POINTS\n");
      for(i=0;i<(long)point_vector.size();i++) {
        fprintf(f," %ld\n",point_vector[i]->id);
      }
    }
    else if (data_type==1) {
      fprintf(f," $POINT_VECTOR\n");
      string ply_file_name = ply_name + PLY_FILE_EXTENSION;//CC
      fprintf(f,"  %s\n",ply_file_name.data());  //TK
  }
  fclose(f);
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
11/2003 OK/CC Implementation
**************************************************************************/
bool CGLPolyline::PointExists(CGLPoint* point,CGLPoint* point1)
{
  long i;
  double dist;
  long dist2;
  long ply_points_vector_length = (long)point_vector.size();
  for (i=0;i<ply_points_vector_length;i++) {
    dist2 = ((point_vector[i]->x_pix - point->x_pix)*
             (point_vector[i]->x_pix - point->x_pix) \
          +  (point_vector[i]->y_pix - point->y_pix)*
             (point_vector[i]->y_pix - point->y_pix));
    dist = sqrt((double)dist2);
    if(dist<point->circle_pix) {
      point1->x_pix = point_vector[i]->x_pix;
      point1->y_pix = point_vector[i]->y_pix;
      point1->x = point_vector[i]->x;
      point1->y = point_vector[i]->y;
      point1->z = point_vector[i]->z;
      return true;
    }
  }
  return false;
}
/**************************************************************************
GeoLib-Method: CenterPoint
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
CGLPoint* CGLPolyline::CenterPoint(void)
{
  CGLPoint *m_point = NULL;
  int polyline_point_vector_length =(long)point_vector.size();
  if(polyline_point_vector_length==0)
    return NULL;
  int center_point = (int)polyline_point_vector_length/2;
  m_point = point_vector[center_point];
  return m_point;
}
/**************************************************************************
GeoLib-Method:
Task: 
Programing:
03/2004 OK Implementation
01/2005 OK File handling
08/2005 CC
**************************************************************************/
void CGLPolyline::WritePointVector(string base)
{
  string delimiter(" ");
  string ply_path;
  string ply_path_base_type;
  long i;
  //----------------------------------------------------------------------
  long no_points =(long)point_vector.size();
  if(no_points>0) {
    //----------------------------------------------------------------------
    // File handling
#ifdef MFC
    CGSProject* m_gsp = GSPGetMember("gli");
    if(m_gsp)
      ply_path = m_gsp->path; 
#else
#endif
    ply_file_name = ply_name + PLY_FILE_EXTENSION;//CC
    ply_path_base_type = ply_path + ply_file_name;
    fstream ply_file (ply_path_base_type.data(),ios::trunc|ios::out);
    ply_file.setf(ios::scientific,ios::floatfield);
	ply_file.precision(12);
    //--------------------------------------------------------------------
    if (!ply_file.good()) return;
    ply_file.seekg(0L,ios::beg);
    for(i=0;i<no_points;i++) {
      ply_file \
        << point_vector[i]->x << delimiter \
        << point_vector[i]->y << delimiter \
        << point_vector[i]->z << endl;
    }
  }
}

/**************************************************************************
GeoLib-Method:
Task: 
Programing:
03/2004 OK Implementation
08/2005 CC file_path
10/2005 OK Path
**************************************************************************/
void CGLPolyline::ReadPointVector(string base)
{
  string cut_string;
  string delimiter_type(" ");
  char line[MAX_ZEILEN];
  string line_string;
  string sub_string;
  string buffer;
  CGLPoint *m_point = NULL;
  //----------------------------------------------------------------------
  // File handling
#ifdef MFC
  CGSProject *m_gsp = GSPGetMember("gli");
  ply_file_name = m_gsp->path + base;
#else
  ply_file_name = base;
#endif
  ifstream ply_file (ply_file_name.data(),ios::in);
  if (!ply_file.good()){
    cout << "Warning in CGLPolyline::ReadPointVector: File not found" << endl;
#ifdef MFC
    AfxMessageBox("Warning in CGLPolyline::ReadPointVector: File not found");
#endif
    return;
  }
  ply_file.seekg(0L,ios::beg);
  //----------------------------------------------------------------------
  while (!ply_file.eof()) {
    ply_file.getline(line,MAX_ZEILEN);
    buffer = line;
    if(buffer.size()==0) continue;
    m_point = new CGLPoint;
    //
    sub_string = get_sub_string2(buffer,delimiter_type,&cut_string);
    m_point->x = strtod(sub_string.data(),NULL);
    buffer = cut_string;
    //
    sub_string = get_sub_string2(buffer,delimiter_type,&cut_string);
    m_point->y = strtod(sub_string.data(),NULL);
    buffer = cut_string;
    //
    sub_string = get_sub_string2(buffer,delimiter_type,&cut_string);
    m_point->z = strtod(sub_string.data(),NULL);
    buffer = cut_string;
    //
    point_vector.push_back(m_point);
  }
}

/**************************************************************************
GeoLib-Method: ComputeLines
Task: 
Programing:
11/2003 WW Implementation
01/2004 CC Modification -- remove orientation value
01/2005 CC Modification -- remove polyline->closed part
**************************************************************************/
void CGLPolyline::ComputeLines(CGLPolyline *m_polyline)
{
  long i;
  CGLPoint *m_point1 = NULL;
  CGLPoint *m_point2 = NULL;
  CGLLine *m_line_exist = NULL;
  CGLLine *m_line2 = NULL;
  // 1 creating all lines from this polyline
  // 2 put lines into polyline line
  long number_of_polyline_points = (long)m_polyline->point_vector.size();
  if (!m_polyline->computeline)
  {
  for(i=1;i<number_of_polyline_points;i++)
  {
    m_point1 = m_polyline->point_vector[i-1];
    m_point2 = m_polyline->point_vector[i];      
    CGLLine *m_line = new CGLLine;
    m_line->point1 = m_point1->id;
    m_line->point2 = m_point2->id;
    m_line->m_point1 = m_point1;
    m_line->m_point2 = m_point2;

    m_line_exist = m_line->Exists();
    if(m_line_exist) {
      m_polyline->line_vector.push_back(m_line_exist);//CC

    }
    else {
      m_line2 = m_line;
      gli_lines_vector.push_back(m_line2);
	  //m_line2->line_index =(long)gli_lines_vector.size();
	  m_line2->gli_line_id = (long)gli_lines_vector.size();
      m_line2->orientation = 1;
      m_polyline->line_vector.push_back(m_line2);//CC
    }
  }

    m_polyline->computeline = true;
  }
}
/**************************************************************************
GEOLib-Method: 
Task: polyline read function
Programing:
03/2004 CC Implementation
05/2004 CC Modification
04/2005 CC Modification calculate the minimal distance between points reference for 
           mesh density of line element calculation
07/2005 CC read ID of polyline
08/2005 CC parameter
**************************************************************************/
void GEOReadPolylines (string file_name_path_base)
{
  CGLPolyline *m_polyline = NULL;
  char line[MAX_ZEILEN];
  string sub_line;
  string line_string;
  string gli_file_name;
  string path_name;
  ios::pos_type position;
#ifdef MFC
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_str_counter;
  CString m_str;
  long counter = 0;
#endif
  //========================================================================
  // File handling
  gli_file_name = file_name_path_base + ".gli";
//  pos = (int)file_name_path_base.rfind('\\'); //CC remove
  //path_name = file_name_path_base.substr(0,(pos+1));//CC remove
  ifstream gli_file (gli_file_name.data(),ios::in);
  if (!gli_file.good()) return;
  gli_file.seekg(0L,ios::beg); // rewind?
  //========================================================================
  // Keyword loop
  while (!gli_file.eof()) {
    gli_file.getline(line,MAX_ZEILEN);
    line_string = line;
    //----------------------------------------------------------------------
    if(line_string.find("#POLYLINE")!=string::npos) { // keyword found
#ifdef MFC
        if(counter==1000){
          m_str = "Read GEO data: Polylines: ";
          m_str_counter.Format("%ld",(long)polyline_vector.size());
          m_str += m_str_counter;
          pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_str);
          counter = 0;
        }
        counter++;
#endif
      m_polyline = new CGLPolyline();
      m_polyline->AssignColor(); //CC
      position = m_polyline->Read(&gli_file,file_name_path_base);//CC8888
      polyline_vector.push_back(m_polyline);
      gli_file.seekg(position,ios::beg);
// OK->CC encapsulate function
      //..................................................................
      m_polyline->CalcMinimumPointDistance();
      //..................................................................
    } // keyword found
  } // eof
  gli_file.close(); //OK41
}

/**************************************************************************
GeoLib-Method: Read
Task: Read polyline data from file
Programing:
03/2004 CC Implementation
09/2004 OK file path for PLY files
07/2005 CC PLY id
08/2005 CC parameter
09/2005 CC itoa - convert integer to string
**************************************************************************/
ios::pos_type CGLPolyline::Read(ifstream *gli_file,string file_path_base)//CC8888
{
  char line[MAX_ZEILEN];
  
  string sub_line;
  string line_string;
  string delimiter(",");
  bool new_keyword = false;
  string hash("#");
  ios::pos_type position;
  string sub_string;
  string tin_file_name;
  CGLPoint *m_point = NULL;
  string delimiter_file_extension(".");
  string cut_string;
  //----------------------------------------------------------------------
  int pos;
  pos = (int)file_path_base.find_last_of('\\');
  string file_path_base_name = file_path_base.substr(0,pos);
  string file_path = file_path_base_name + "\\" ;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while (!new_keyword) {
    position = gli_file->tellg();
    gli_file->getline(line,MAX_ZEILEN);
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    if(line_string.find("$ID")!=string::npos) { // subkeyword found CC
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      remove_white_space(&line_string);
      id = strtol(line_string.data(),NULL,0);

      continue;
    } 
    //....................................................................
    if(line_string.find("$NAME")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      remove_white_space(&line_string);
      name = line_string.substr(0);
      continue;
    } // subkeyword found
    //....................................................................
    if(line_string.find("$TYPE")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      type = strtol(line_string.data(),NULL,0);
      continue;
    } // subkeyword found
    //....................................................................
    if(line_string.find("$EPSILON")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      remove_white_space(&line_string);
      epsilon = strtod(line_string.data(),NULL);
      continue;
    } // subkeyword found
    //....................................................................
    if(line_string.find("$MAT_GROUP")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      mat_group = strtol(line_string.data(),NULL,0);
      continue;
    } // subkeyword found
    //....................................................................
    if(line_string.find("$POINTS")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      ply_type = "GEO_POINTS";//CC8888
      ply_data = "POINTS";//CC8888
      long lvalue = strtol(line_string.data(),NULL,0);
      int i = 1;
      while(i == 1) {
        m_point = GEOGetPointById(lvalue);//CC wrong get point by id
        if(m_point)
          AddPoint(m_point); 
         //CC---------------------------
        else{
          cout << "Error: point " << lvalue << " not found" << endl;
          //--------------------------------------------------
         string m_strname = name + ": Point not found: point "; 
         char m_strnameid[10];
         sprintf(m_strnameid,"%li",lvalue); //OK
         //itoa((int)lvalue,m_strnameid,10); //CC 09/05 convert integer to string ultoa convert unsigned long to string
         string error_str = m_strname + m_strnameid;
#ifdef MFC
         AfxMessageBox(error_str.c_str());
#endif
         return position;
        };
        gli_file->getline(line,MAX_ZEILEN);
        line_string = line;
        if((line_string.find(hash)!=string::npos)\
         ||(line_string.find("$")!=string::npos)){ //OK bugfix
            i = 0;
            new_keyword = true;
          }
        else
           lvalue = strtol(line_string.data(),NULL,0);
       }//end of while
      data_type = 0;
    } // subkeyword found
    //....................................................................
    if(line_string.find("$POINT_VECTOR")!=string::npos) { // subkeyword found
      gli_file->getline(line,MAX_ZEILEN);
      line_string = line;
      ply_type = "NOD_POINTS";//CC8888
      remove_white_space(&line_string);
      ply_file_name = line_string.substr(0);
      ReadPointVector(ply_file_name);//CC
      data_type = 1; //OK41
      ply_data = line_string;
    } // subkeyword found
  //========================================================================
  }
  return position;
}

/**************************************************************************
GeoLib-Method:addpoint
Task: 
Programing:
03/2004 CC Implementation
**************************************************************************/
void CGLPolyline::AddPoint(CGLPoint* m_point)
{
	 //point_list.push_back(m_point);//CC remove
     point_vector.push_back(m_point);
}
/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
05/2004 CC Implementation Assign randam color to polyline
last modification:
**************************************************************************/
void CGLPolyline::AssignColor()
{
    long r_number = rand();
    m_color[0] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_color[1] = (long)(r_number/(double)RAND_MAX*256);
    r_number = rand();
    m_color[2] = (long)(r_number/(double)RAND_MAX*256);
}



/**************************************************************************
FEMLib-Method: InterpolationAlongPolyline
Task: Prescibe the boundary values to all points of a polyline by the means 
of spline
Programing:
02/2004 WW Implementation
last modification:
**************************************************************************/
void InterpolationAlongPolyline(CGLPolyline *plyL, 
                                vector<double>& bcNodalValue)  
{
   long number_of_nodes, i;

   double sl = 0.0;
     
   number_of_nodes = (long)bcNodalValue.size();

   vector<double> ss0;
   vector<double> bVal;

   //Obtain fem node for groupvector
   CGLPoint *CGPa=NULL, *CGPb=NULL;
   const int SizeCGLPoint = (int)plyL->point_vector.size();
   double xp,yp,zp;
   xp = yp = zp = 0.0;


   // Prepare spline data 
   sl = 0.0;
   for(i=0; i<SizeCGLPoint-1; i++)
   {
	   CGPa = plyL->point_vector[i];
       CGPb = plyL->point_vector[i+1];
       xp = CGPb->x-CGPa->x;
	   yp = CGPb->y-CGPa->y;
	   zp = CGPb->z-CGPa->z;
    if(fabs(plyL->point_vector[i]->property)> MKleinsteZahlen)
       {
	  bVal.push_back(plyL->point_vector[i]->property);
          ss0.push_back(sl);
       } 
       sl += sqrt(xp*xp+yp*yp+zp*zp);
   }

   // Last point
  if(fabs(plyL->point_vector[SizeCGLPoint-1]->property)> MKleinsteZahlen)
   {
       ss0.push_back(sl);
    bVal.push_back(plyL->point_vector[SizeCGLPoint-1]->property);
   }

   // Spline interpolation
   CubicSpline *csp = new CubicSpline(ss0, bVal);

   // Interpolate
   for(i=0; i<number_of_nodes; i++)
	   bcNodalValue[plyL->OrderedPoint[i]] = csp->interpolation(plyL->sbuffer[i]); 

   // Release the memory
   delete csp;
   csp = NULL;
   ss0.clear();
   bVal.clear();
 //  OrderedNode.clear();
 //  plyL->sbuffer.clear();
 //  plyL->ibuffer.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
08/2005 CC file_path
last modification:
**************************************************************************/
void CGLPolyline::WriteTecplot(string file_path)
{

  long i;
  //----------------------------------------------------------------------
  // File handling
  string tec_path;
 /* CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp)
    tec_path = m_gsp->path; */
 
  string tec_file_name = file_path + name + ".tec";
  //string tec_file_name = tec_path + name + ".tec";
  fstream tec_file (tec_file_name.data(),ios::trunc|ios::out);
  tec_file.setf(ios::scientific,ios::floatfield);
  tec_file.precision(12);
  // Write header
  tec_file << "VARIABLES = X,Y,Z" << endl;
  long no_nodes = (long)point_vector.size();
  long no_elements = no_nodes-1;
  tec_file << "ZONE T = " << name << ", " \
             << "N = " << no_nodes << ", " \
             << "E = " << no_elements << ", " \
             << "F = FEPOINT" << ", " << "ET = TRIANGLE" << endl;
  // Write data
  for(i=0;i<no_nodes;i++) {
    tec_file \
      << point_vector[i]->x << " " << point_vector[i]->y << " " << point_vector[i]->z << " " \
      << endl;
  }
  for(i=0;i<no_elements;i++) {
    tec_file \
      << i+1 << " " << i+1+1 << " " << i+1 << endl;
  }

}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGLPolyline::SortPointVectorByDistance()
{
  long no_points = (long)point_vector.size();
  if(no_points==0)
    return;
  long i;
  double pt1[3],pt2[3];
  long* nodes_sorted = NULL;
  long* nodes_unsorted = new long[no_points];
  for(i=0;i<no_points;i++) {
    nodes_unsorted[i] = i;
  }
  pt1[0] = point_vector[0]->x;
  pt1[1] = point_vector[0]->y;
  pt1[2] = point_vector[0]->z;
  double *node_distances = new double[no_points];
  for(i=1;i<no_points;i++) {
    pt2[0] = point_vector[i]->x;
    pt2[1] = point_vector[i]->y;
    pt2[2] = point_vector[i]->z;
    node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
  }
  nodes_sorted = TOLSortNodes1(nodes_unsorted,node_distances,no_points);
  // Reorder point vector
  vector<CGLPoint*>aux_point_vector;
  CGLPoint* m_pnt;
  for(i=0;i<no_points;i++){
    m_pnt = point_vector[nodes_sorted[i]];
    aux_point_vector.push_back(m_pnt);
  }
  point_vector.clear();
  point_vector = aux_point_vector;
  // Release memory
  delete [] node_distances;
  delete [] nodes_unsorted;
}
/**************************************************************************
FEMLib-Method: GetOrderedNodeByDistance_Polyline(const long *nodes, 
               const CGLPolyline *plyL, vector<double>& Distance, 
			   vector<int>& OrderedNode)  
Task: Prescibe the boundary values to all points of a polyline by the means 
of spline
          
Programing:
02/2004 WW Implementation
last modification:
23/2004 WW 
08/2005 WW Set as a member of polyline
04/2006 WW Fix a big for polyines have more than two points
**************************************************************************/
void CGLPolyline::GetPointOrderByDistance()  
{
   long number_of_nodes, i, l;
   int j, k;

   double sl = 0.0, s0=0.0;
   double xp, yp,zp;

   number_of_nodes = (long)ibuffer.size();
   OrderedPoint.resize(number_of_nodes);
   //Obtain fem node for groupvector
   CGLPoint *CGPa=NULL, *CGPb=NULL;
   const int SizeCGLPoint = (int)point_vector.size();

   // Reorder the node
   for(j=0; j<number_of_nodes; j++)
      OrderedPoint[j] = j;

   // Reorder the nodes finded along polyline 
   /*
   for(i=0; i<SizeCGLPoint-1; i++)
   {
	   // Reorder the node
       for(j=0; j<number_of_nodes; j++)
       {
          if(ibuffer[j]!=i) continue;
          for(k=j; k<number_of_nodes; k++)
		  {
             if(ibuffer[k]!=i) continue;
             s0 = sbuffer[j];
             sl = sbuffer[k];		  
             if(sl<s0)
			 {
                l = OrderedPoint[j];
                sbuffer[k] = s0;
                sbuffer[j] = sl;
 				OrderedPoint[j] = OrderedPoint[k];
                OrderedPoint[k] = l;
			 }
		  }
	   }
   }
   */

   // Reorder the nodes within a sector
   for(j=0; j<number_of_nodes; j++)
   {
       i=ibuffer[j];
       for(k=j; k<number_of_nodes; k++)
       {
          if(ibuffer[k]!=i) continue;
          s0 = sbuffer[j];
          sl = sbuffer[k];		  
          if(sl<s0)
          {
             l = OrderedPoint[j];
             sbuffer[k] = s0;
             sbuffer[j] = sl;
             OrderedPoint[j] = OrderedPoint[k];
             OrderedPoint[k] = l;
          }
	   }
   }

   sl = 0.0;
   for(i=0; i<SizeCGLPoint-1; i++)
   {
       for(j=0; j<number_of_nodes; j++)
	   {
          if(ibuffer[j]==i)
             sbuffer[j] += sl; // distance to the first point of the polyline
	   }
	   CGPa = point_vector[i];
       CGPb = point_vector[i+1];
       xp = CGPb->x-CGPa->x;
	   yp = CGPb->y-CGPa->y;
	   zp = CGPb->z-CGPa->z;
       sl += sqrt(xp*xp+yp*yp+zp*zp);       
   }
   //
   // Reorder all nodes found 
   for(j=0; j<number_of_nodes; j++)
   {
       for(k=j; k<number_of_nodes; k++)
       {
          if(k==j) continue;
          s0 = sbuffer[j];
          sl = sbuffer[k];		  
          if(sl<s0)
          {
             l = OrderedPoint[j];
             sbuffer[k] = s0;
             sbuffer[j] = sl;
             OrderedPoint[j] = OrderedPoint[k];
             OrderedPoint[k] = l;
          }
	   }
   }
}

/**************************************************************************
GEOLib-Method: 
Task: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void GEOUnselectPLY()
{
  CGLPolyline* m_ply = NULL;
  vector<CGLPolyline*>::const_iterator p_ply; 
  p_ply = polyline_vector.begin();
  while(p_ply!=polyline_vector.end()) {
    m_ply = *p_ply;
    m_ply->highlighted = false;
    ++p_ply;
  }
}

/**************************************************************************
GEOLib-Method
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGLPolyline::SetPointOrderByDistance(CGLPoint*m_pnt)  
{
  int i;
//OK  double eps = 1e-3;
  //----------------------------------------------------------------------
  CGLPoint* m_pnt_i = NULL;
  vector<CGLPoint*>point_vector_aux;
  for(i=0;i<(int)point_vector.size();i++){
    m_pnt_i = point_vector[i];
    //OK if(m_pnt->PointDisXY(m_pnt_i)<epsilon){
    if(m_pnt->PointDis(m_pnt_i)<epsilon){
      point_vector_aux.push_back(m_pnt_i);
    }
  }
  if((int)point_vector_aux.size()==0){
#ifdef MFC
    AfxMessageBox("Error in CGLPolyline::SetPointOrderByDistance: no PNT data");
#endif
    return;
  }
  for(i=0;i<(int)point_vector.size();i++){
    m_pnt_i = point_vector[i];
    if(m_pnt_i==point_vector_aux[0])
      continue;
    point_vector_aux.push_back(m_pnt_i);
  }
  point_vector.clear();
  point_vector = point_vector_aux;
  SortPointVectorByDistance();
/*
  //----------------------------------------------------------------------
  sbuffer.clear();
  ibuffer.clear();
  CGLPoint* m_pnt_0 = point_vector[0];
  for(i=0;i<(int)point_vector.size();i++){
    m_pnt_i = point_vector[i];
    ibuffer.push_back(i);
    dist = m_pnt_i->PointDisXY(m_pnt_0);
    sbuffer.push_back(dist);
  }
  //----------------------------------------------------------------------
  GetPointOrderByDistance();
*/
 //----------------------------------------------------------------------
}
/**************************************************************************
GEOLib-Method: 
Task: 
Programing:
11/2005 CC Implementation
**************************************************************************/
void GEOWritePolylines(char* file_name)
{
  CGLPolyline* m_ply = NULL;
  for (int i =0; i <(int)polyline_vector.size();i++)
  {
    m_ply = polyline_vector[i];
    m_ply->Write(file_name);
  }
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void GEORemovePLY(CGLPolyline*m_ply)
{
  CGLPolyline* m_ply_this = NULL;
  vector<CGLPolyline*>::const_iterator p_ply = polyline_vector.begin();
  for(int i=0;i<(int)polyline_vector.size();i++){
    m_ply_this = polyline_vector[i];
    if(m_ply_this->name.compare(m_ply->name)==0){ 
      delete m_ply_this;
      polyline_vector.erase(polyline_vector.begin()+i);
      //i--;
      return;
    }
  }
}

/**************************************************************************
GeoLib-Method: 
01/2006 OK Implementation based on CCs version
**************************************************************************/
void CGLPolyline::CalcMinimumPointDistance()
{
  CGLPoint* start_point = NULL;
  CGLPoint* end_point = NULL;
  double m_dXMin = 1.e+19;
  double min;
  for(int i=0;i<(int)point_vector.size()-1;i++)
  {
    start_point = point_vector[i];
    end_point = point_vector[i+1];
    //min = sqrt((start_point->x-end_point->x)*(start_point->x-end_point->x)+(start_point->y-end_point->y)*(start_point->y-end_point->y));
    min = start_point->PointDis(end_point);
    if(min<m_dXMin)
      m_dXMin = min;
  }
  minDis = m_dXMin;
}
