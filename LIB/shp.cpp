#include "stdafx.h"

#include "makros.h"
#include <math.h>

// GeoLib
#include "geo_pnt.h"
#include "geo_ply.h"

// ShpLib
#include "shapefil.h"

#define SHP_INPUT_EXTENSION  ".shp"



/*************************************************************************
  ROCKFLOW - Function: SHPReadFile
  Task: read ArcView shape files
  Programming: 07/2003 CC
  last modified:11/2003 CC
  01/2005 CC Modification surface and point_vector
  04/2005 CC Modification calculate minimal distances between points of pol
**************************************************************************/
int SHPReadFile(char *dateiname)
{
  //control variables
  int checklap(1);
  bool overlap = FALSE;
 double m_dXMin;
  vector<CGLPoint*> d_points_vector;
  long number_of_points = 0;
  //int point_number = 0;
  
  int datlen;
  char *shp_file = NULL;
  char shapetype[MAX_ZEILE];
  char string_name[MAX_ZEILE];
  char string_pntname[MAX_ZEILE];
  //GeoLib objects
  CGLPoint *m_point = NULL;
  CGLPolyline *polyline = NULL;

  CGLPoint* start_point = NULL;
  CGLPoint* end_point = NULL;
  vector<CGLPoint*> gs_points_vector;
  vector<CGLPolyline*> gs_polyline_vector;
  //check variables
  long number_of_point = 0;
  long number_of_ply = 0;
  // shape objects
  SHPHandle hSHP;
  SHPObject *hSHPObject;
  datlen = (int)strlen(dateiname) + 5;
  shp_file = (char *) Malloc(datlen);
 // shp_file = new char[datlen];//CC
 // shp_file = new char[datlen +1];
  shp_file = strcat(strcpy(shp_file,dateiname),SHP_INPUT_EXTENSION);
  //open shape files;
  hSHP = SHPOpen (shp_file,"rb");
  long nRecord;
  nRecord=(long)hSHP->nRecords;



 //read the shape files 
  if (hSHP){
    for(int i=0;i<=(nRecord-1);i++)
  {   
        
    hSHPObject=SHPReadObject(hSHP,i);

//2-D shapefile type:(ArcView 3.x)
	//CC14.01.2004
         switch (hSHPObject->nSHPType)
         {
          case 1: sprintf(shapetype,"%s","POINT");

			    //check existing gli_points_vector;
                  
				  
				  //sprintf(shapetype,"%s","POINT");
			      number_of_point = (long)gli_points_vector.size();//CC
                  sprintf(string_name,"%s%d",shapetype,number_of_point);
                  
                  //m_point = m_point->Create(string_name);
                  m_point = new CGLPoint();
                 // m_point->name = (string)string_name;
                    m_point->name = string_name;
                  m_point->highlighted = false; //CC
                  gli_points_vector.push_back(m_point);
                  //m_point->id = (long)i;
                  m_point->id = number_of_point;
                  m_point->x = *(hSHPObject->padfX);
                  m_point->y = *(hSHPObject->padfY);
                  m_point->z = *(hSHPObject->padfZ);
//                  m_point->mesh_density = 1;
//				  m_point->type = 0;
                
                  
				  break;
          case 3: sprintf(shapetype,"%s","POLYLINE");
				  
				  //sprintf(shapetype,"%s","POLYLINE");
                  gs_polyline_vector = GetPolylineVector();//CC
                  number_of_ply = (long)gs_polyline_vector.size();
			      sprintf(string_name,"%s%d",shapetype,number_of_ply);
                 // polyline = polyline->Create(string_name);//CC 08/05
                  polyline = new CGLPolyline;
                  polyline->name = string_name;
                  polyline_vector.push_back(polyline);
                  polyline->id = number_of_ply;
		 
		          for(int j=0;j<=(hSHPObject->nVertices-1);j++)         
			      {
                  //sprintf(string_pntname,"%s%d","POINT",0);
			      //m_point = m_point->Create(string_pntname);
				  CGLPoint* m_point2 = NULL;
				  CGLPoint* m_point1 = NULL;
                  m_point= new CGLPoint;
                  m_point->x_pix = 0;
                  m_point->y_pix = 0;
                  m_point->x = *(hSHPObject->padfX+j);
                  m_point->y = *(hSHPObject->padfY+j);
		          m_point->z = *(hSHPObject->padfZ+j);
			      //m_point->gli_point_id = (long)j;
				  //m_point->mesh_density = 0;
                  gs_points_vector = GetPointsVector();//CC
				  vector<CGLPoint*>::iterator p = gs_points_vector.begin();
				  while(p!=gs_points_vector.end())
				  {
				  m_point2 = *p;
                  double valuemin = m_point->PointDis(m_point2);
				  if(valuemin < MKleinsteZahl)
				  {
					  overlap = TRUE;
                      checklap = 0;
					 
				      break;
				  }
				  ++p;
				  }
				  if(overlap == FALSE)
				  {
			    //check gli_points_vector;
				   number_of_point = (long)gli_points_vector.size();//CC
                   sprintf(string_pntname,"%s%d","POINT",number_of_point);
                   m_point1 = new CGLPoint();
                   m_point1->name = string_pntname;
                   m_point1->highlighted = false; //CC
                   gli_points_vector.push_back(m_point1);
				   m_point1->id = number_of_point;
				  
				   m_point1->x = m_point->x;
				   m_point1->y = m_point->y;
				   m_point1->z = m_point->z;
				   m_point1->x_pix = 0;
				   m_point->y_pix = 0;
				   m_point1->mesh_density = m_point->mesh_density;
				   //CC/OK m_point1->type = m_point->type;
                  // polyline->point_list.push_back(m_point1);//CC remove
                   polyline->point_vector.push_back(m_point1);
                   //++point_number;
				   checklap = 1;
				  }
				  
				  if (checklap == 0)
				  {
                  //polyline->point_list.push_back(m_point2);//CC remove
                  polyline->point_vector.push_back(m_point2);
				  overlap = FALSE;
				  }		

                
				  //                 
                  }// end of for
                  //04/2005 calculate minimal distance
                  m_dXMin = 1.e+19;  
		          d_points_vector = polyline->point_vector;
                  number_of_points = (long)d_points_vector.size();
                 
                 
                  for (int i = 0;i<number_of_points-1;i++)
                  {
                  start_point = d_points_vector[i];
                  end_point = d_points_vector[i+1];
                  double min = sqrt((start_point->x-end_point->x)*(start_point->x-end_point->x)+(start_point->y-end_point->y)*(start_point->y-end_point->y));
                  if(min<m_dXMin)
                     m_dXMin = min;
                  }
                  polyline->minDis = m_dXMin;        

                 
				  break;
         case 5:  sprintf(shapetype,"%s","POLYLINE");
			      gs_polyline_vector = GetPolylineVector();//CC
                  number_of_ply = (long)gs_polyline_vector.size();//CC
			      sprintf(string_name,"%s%d",shapetype,number_of_ply);
                 // polyline = polyline->Create(string_name);
                  polyline = new CGLPolyline;
                  polyline->name = string_name;
                  polyline_vector.push_back(polyline);
                  polyline->closed = TRUE;
                  polyline->id = number_of_ply;

		 
		          for(int j=0;j<=(hSHPObject->nVertices-1);j++)         
			      {
            
				  CGLPoint* m_point2 = NULL;
				  CGLPoint* m_point1 = NULL;
                  m_point= new CGLPoint;
                  m_point->x_pix = 0;
                  m_point->y_pix = 0;
                  m_point->x = *(hSHPObject->padfX+j);
                  m_point->y = *(hSHPObject->padfY+j);
		          m_point->z = *(hSHPObject->padfZ+j);
			     
                  gs_points_vector = GetPointsVector();//CC
				  vector<CGLPoint*>::iterator p = gs_points_vector.begin();
				  while(p!=gs_points_vector.end())
				  {
				  m_point2 = *p;
                  double valuemin = m_point->PointDis(m_point2);
				  if(valuemin < MKleinsteZahl)
				  {
					  overlap = TRUE;
                      checklap = 0;
					 
				      break;
				  }
				  ++p;
				  }
				  if(overlap == FALSE)
				  {
			    //check gli_points_vector;
				   number_of_point = (long)gli_points_vector.size();//CC
                   sprintf(string_pntname,"%s%d","POINT",number_of_point);
                   m_point1 = new CGLPoint();
                   m_point1->name = string_pntname;
                   m_point1->highlighted = false; //CC
                   gli_points_vector.push_back(m_point1);
				   m_point1->id = number_of_point;
				  
				   m_point1->x = m_point->x;
				   m_point1->y = m_point->y;
				   m_point1->z = m_point->z;
				   m_point1->x_pix = 0;
				   m_point->y_pix = 0;
				   m_point1->mesh_density = m_point->mesh_density;
//CC/OK 				   m_point1->type = m_point->type;
                  // polyline->point_list.push_back(m_point1);//CC remove
                   polyline->point_vector.push_back(m_point1);
                   //++point_number;
				   checklap = 1;
				  }
				  
				  if (checklap == 0)
				  {
                 // polyline->point_list.push_back(m_point2);//CC remove
                  polyline->point_vector.push_back(m_point2);
				  overlap = FALSE;
				  }				 
				  //  
                  //long aa =(long)polyline->point_vector.size();
                  }

                  //CC 04/05 min distance
                    m_dXMin = 1.e+19;  
		          d_points_vector = polyline->point_vector;
                  number_of_points = (long)d_points_vector.size();
                 
                 
                  for (int i = 0;i<number_of_points-1;i++)
                  {
                  start_point = d_points_vector[i];
                  end_point = d_points_vector[i+1];
                  double min = sqrt((start_point->x-end_point->x)*(start_point->x-end_point->x)+(start_point->y-end_point->y)*(start_point->y-end_point->y));
                  if(min<m_dXMin)
                     m_dXMin = min;
                  }
                  polyline->minDis = m_dXMin;  
                  break;
          case 11: sprintf(shapetype,"%s","PointZ");
                   
				  //sprintf(shapetype,"%s","POINT");
			      number_of_point = (long)gli_points_vector.size();//CC
                  sprintf(string_name,"%s%d",shapetype,number_of_point);
                  
                   m_point = new CGLPoint();
                   m_point->name = string_name;
                   m_point->highlighted = false; //CC
                   gli_points_vector.push_back(m_point);
                   m_point->id = number_of_point;
                   m_point->x = *(hSHPObject->padfX);
                   m_point->y = *(hSHPObject->padfY);
                   m_point->z = *(hSHPObject->padfZ);

                  break;
           case 15:  sprintf(shapetype,"%s","PolygonZ");
			      gs_polyline_vector = GetPolylineVector();//CC
                  number_of_ply = (long)gs_polyline_vector.size();//CC
			      sprintf(string_name,"%s%d",shapetype,number_of_ply);
                  //polyline = polyline->Create(string_name);//CC
                  polyline = new CGLPolyline;
                  polyline->name = string_name;
                  polyline_vector.push_back(polyline);
                  polyline->closed = TRUE;
                  polyline->id = number_of_ply;

		 
		          for(int j=0;j<=(hSHPObject->nVertices-1);j++)         
			      {
                  //sprintf(string_pntname,"%s%d","POINT",0);
			      //m_point = m_point->Create(string_pntname);
				  CGLPoint* m_point2 = NULL;
				  CGLPoint* m_point1 = NULL;
                  m_point= new CGLPoint;
                  m_point->x_pix = 0;
                  m_point->y_pix = 0;
                  m_point->x = *(hSHPObject->padfX+j);
                  m_point->y = *(hSHPObject->padfY+j);
		          m_point->z = *(hSHPObject->padfZ+j);
			     
                  gs_points_vector = GetPointsVector();//CC
				  vector<CGLPoint*>::iterator p = gs_points_vector.begin();
				  while(p!=gs_points_vector.end())
				  {
				  m_point2 = *p;
                  double valuemin = m_point->PointDis(m_point2);
				  if(valuemin < MKleinsteZahl)
				  {
					  overlap = TRUE;
                      checklap = 0;
					 
				      break;
				  }
				  ++p;
				  }
				  if(overlap == FALSE)
				  {
			    //check gli_points_vector;
				   number_of_point = (long)gli_points_vector.size();//CC
                   sprintf(string_pntname,"%s%d","POINT",number_of_point);
                   m_point1 = new CGLPoint();
                   m_point1->name = string_pntname;
                   m_point1->highlighted = false; //CC
                   gli_points_vector.push_back(m_point1);
				   m_point1->id = number_of_point;
				  
				   m_point1->x = m_point->x;
				   m_point1->y = m_point->y;
				   m_point1->z = m_point->z;
				   m_point1->x_pix = 0;
				   m_point->y_pix = 0;
				   m_point1->mesh_density = m_point->mesh_density;
//CC/OK 				   m_point1->type = m_point->type;
                  // polyline->point_list.push_back(m_point1);//CC remove
                   polyline->point_vector.push_back(m_point1);
                   //++point_number;
				   checklap = 1;
				  }
				  
				  if (checklap == 0)
				  {
                  //polyline->point_list.push_back(m_point2);//CC remove
                  polyline->point_vector.push_back(m_point2);
				  overlap = FALSE;
				  }				 
				  //  
                  //long aa =(long)polyline->point_vector.size();
                  }
                  break;
          default: AfxMessageBox("undeclared shapefile type!",MB_OK,0);
		 }
    
	 }
   }

  shp_file = (char *) Free (shp_file);
  shp_file = NULL;
 
  SHPClose(hSHP);

  return 0;
}