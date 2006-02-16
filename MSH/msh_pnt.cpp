/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C
#include "math.h"
// C++
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_elements_rfi.h"
// PCSLib

/**************************************************************************
MshLib-Method: SearchElementsWithPointInCircumsphere
Task: liest den Wert und sucht alle Elemente raus die den angegebenen Punkt
im Umkreis haben
Programing:
05/2004 TK Implementation
**************************************************************************/
void SearchElementsWithPointInCircumsphere(double x, double y, double z)
{
	int i=0;
	long number_of_all_elements;
	long inout=0;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (long)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
	   inout = PointInsideTriangleCircumsphere(x,y,z,i);
	   if (inout == 1) element_vector[i]->element_marker = TRUE;
	   else element_vector[i]->element_marker = FALSE;
	}

	
/**************************************************************************/
/**************************************************************************/
/*Schreiben des GeoFiles um die Umkreise zu prüfen*/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

 FILE *geo_file=NULL;
 const char *file_name = "C:\\GSProject\\Circle.geo";
 geo_file = fopen(file_name, "w+t");
	
	/*Points:*/     
	long number_of_all_nodes;
	int j=0;
	long point0, point1, point2;
   	int line_id_counter = 0;
 	vector<CMSHNodes*> nodes_vector;
	nodes_vector = MSHGetNodeVector();
	number_of_all_nodes	   = (long)nodes_vector.size();
	number_of_all_elements = (long)element_vector.size();
	fprintf(geo_file,"%s\n","Geometry.CirclePoints = 50;");
	fprintf(geo_file,"%s\n","//Points");
	for (i=0;i<number_of_all_nodes;i++)
	{ 
		fprintf(geo_file,"%s","Point(");
		fprintf(geo_file,"%i",i);
		fprintf(geo_file,"%s",") = {");
		fprintf(geo_file,"%g",nodes_vector[i]->x);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%g",nodes_vector[i]->y);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%g",nodes_vector[i]->z);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%s"," 0.500000000");
		fprintf(geo_file,"%s\n","};");
	}

	for (j=0;j<number_of_all_elements;j++)
			{
				line_id_counter++;
				point0 = element_vector[j]->node1;
				point1 = element_vector[j]->node2;
				point2 = element_vector[j]->node3;
				fprintf(geo_file,"%s","Line(");
				fprintf(geo_file,"%i",line_id_counter);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%ld",point0);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%ld",point1);
				fprintf(geo_file,"%s\n","};");
				line_id_counter++;
				point0 = element_vector[j]->node1;
				point1 = element_vector[j]->node2;
				point2 = element_vector[j]->node3;
				fprintf(geo_file,"%s","Line(");
				fprintf(geo_file,"%i",line_id_counter);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%ld",point1);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%ld",point2);
				fprintf(geo_file,"%s\n","};");
				line_id_counter++;
				point0 = element_vector[j]->node1;
				point1 = element_vector[j]->node2;
				point2 = element_vector[j]->node3;
				fprintf(geo_file,"%s","Line(");
				fprintf(geo_file,"%i",line_id_counter);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%ld",point2);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%ld",point0);
				fprintf(geo_file,"%s\n","};");
			}

			for (j=0;j<number_of_all_elements;j++)
			{
				line_id_counter++;
           		fprintf(geo_file,"%s","Point(");
				fprintf(geo_file,"%ld",j+number_of_all_nodes);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%g",element_vector[j]->circumcenter_x);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%g",element_vector[j]->circumcenter_y);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%g",element_vector[j]->circumcenter_z);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%s"," 0.500000000");
				fprintf(geo_file,"%s\n","};");
				point0 = element_vector[j]->node1;
				fprintf(geo_file,"%s","Circle(");
				fprintf(geo_file,"%i",line_id_counter);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%ld",point0);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%ld",j+number_of_all_nodes);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%ld",point0);
				fprintf(geo_file,"%s\n","};");
			}

		   		fprintf(geo_file,"%s","Point(");
				fprintf(geo_file,"%ld",number_of_all_elements+number_of_all_nodes+1);
				fprintf(geo_file,"%s",") = {");
				fprintf(geo_file,"%f",x);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%f",y);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%f",z);
				fprintf(geo_file,"%s",", ");
				fprintf(geo_file,"%s"," 0.500000000");
				fprintf(geo_file,"%s\n","};");
		
 fclose(geo_file);

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/




}

/**************************************************************************
MshLib-Method: PointInsideCircumsphere
Input: die Koordinaten eines Punktes und die Vektor-Element-Id. Achtung muss nicht identisch sein 
mit der eingetragenen Element-ID. Auswahl des Elemenzs sollte also über ein Schleifen-Kriterium erfolgen
Task: Gibt Null (0) zurück wenn Punkt außerhalb liegt und eins (1) wenn Punkt auf und in der Umkugel liegt
Programing:
05/2004 TK Implementation
**************************************************************************/
long PointInsideTriangleCircumsphere(double x, double y, double z, long element_id)
{
	double radius, cmid_x, cmid_y, cmid_z, distance;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector= MSHGetElementVector();
	
	radius = element_vector[element_id]->circumsphere_radius;
	cmid_x = element_vector[element_id]->circumcenter_x;
	cmid_y = element_vector[element_id]->circumcenter_y;
	cmid_z = element_vector[element_id]->circumcenter_z;

	//calculates distance:
	distance = sqrt((cmid_x-x)*(cmid_x-x) + (cmid_y-y)*(cmid_y-y) + (cmid_z-z)*(cmid_z-z));
	if (distance <= radius)	return 1;
	else return 0;
}


/**************************************************************************
MshLib-Method:  tricircumcenter3d()   Find the circumcenter of a triangle in 3D
Task: The result is returned both in terms of xyz coordinates                
	  coordinates, relative to the triangle's point `a' (that is, `a' is     
	  the origin of both coordinate systems).  Hence, the xyz coordinates    
	  returned are NOT absolute; one must add the coordinates of `a' to      
	  find the absolute coordinates of the circumcircle.  However, this means
	  that the result is frequently more accurate than would be possible if  
	  absolute coordinates were returned, due to limited floating-point      
	  precision.  In general, the circumradius can be computed much more     
	  accurately.                                                            
Programing:
05/2004 TK Implementation
**************************************************************************/
void TriCircumcenter3d(double a[3],double b[3],double c[3],double circumcenter[3])
{
  double xba, yba, zba, xca, yca, zca;
  double balength, calength;
  double xcrossbc, ycrossbc, zcrossbc;
  double denominator;
  double xcirca, ycirca, zcirca;

  /* Use coordinates relative to point `a' of the triangle. */
  xba = b[0] - a[0];
  yba = b[1] - a[1];
  zba = b[2] - a[2];
  xca = c[0] - a[0];
  yca = c[1] - a[1];
  zca = c[2] - a[2];
  /* Squares of lengths of the edges incident to `a'. */
  balength = xba * xba + yba * yba + zba * zba;
  calength = xca * xca + yca * yca + zca * zca;
  
  /* Cross product of these edges. */
#ifdef EXACT
  /* Use orient2d() from http://www.cs.cmu.edu/~quake/robust.html     */
  /*   to ensure a correctly signed (and reasonably accurate) result, */
  /*   avoiding any possibility of division by zero.                  */
  xcrossbc = orient2d(b[1], b[2], c[1], c[2], a[1], a[2]);
  ycrossbc = orient2d(b[2], b[0], c[2], c[0], a[2], a[0]);
  zcrossbc = orient2d(b[0], b[1], c[0], c[1], a[0], a[1]);
#else
  /* Take your chances with floating-point roundoff. */
  xcrossbc = yba * zca - yca * zba;
  ycrossbc = zba * xca - zca * xba;
  zcrossbc = xba * yca - xca * yba;
#endif

  /* Calculate the denominator of the formulae. */
  denominator = 0.5 / (xcrossbc * xcrossbc + ycrossbc * ycrossbc +
                       zcrossbc * zcrossbc);
  
  /* Calculate offset (from `a') of circumcenter. */
  xcirca = ((balength * yca - calength * yba) * zcrossbc -
            (balength * zca - calength * zba) * ycrossbc) * denominator;
  ycirca = ((balength * zca - calength * zba) * xcrossbc -
            (balength * xca - calength * xba) * zcrossbc) * denominator;
  zcirca = ((balength * xca - calength * xba) * ycrossbc -
            (balength * yca - calength * yba) * xcrossbc) * denominator;
  circumcenter[0] = xcirca + a[0];
  circumcenter[1] = ycirca + a[1];
  circumcenter[2] = zcirca + a[2];

}

