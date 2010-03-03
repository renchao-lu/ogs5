/**************************************************************************
Mesh Quality Test Tools

1. Schritt 4/2004  TK => Utersuchung geometrischer Kriterien

**************************************************************************/
#include "msh_quality.h"
// C/C++/MFC
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
using namespace std;
// constructor
CMSHNodesTest::CMSHNodesTest(void)
{
}
// Vector/List
vector<CMSHElementsTest*> msh_elements_test_vector;
CMSHElementsTest *m_msh_test_elements = NULL;
// constructor
CMSHElementsTest::CMSHElementsTest(void)
{
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**/
/*=*=*=*=*=*=*=*=*=*=*=                   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=* F U N C T I O N S =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*=*=*=*=*=*=*=*=*=*=*=                   =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**/
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**************************************************************************/
/* MshLib - Funktion: CalculateBasicTriangleData
                                                                          */
/* Aufgabe: CalculateBasicTriangleData l
			geht durch das Datenkonstrukt und berechnet die Vektorlängen Winkel Fläche
			und schreibt die entsprechende Werte in das Konstrukt
			berechnete und geschrieben Werte: |a|&|b|&|c|& alpha & beta & gamma & Fläche A
											  QA (area quality)
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateBasicTriangleData()
{
	long number_of_all_elements = 0;
	int i=0;
	double check_value;
	double pi = 3.14159265359;
	double a[3];
	double b[3];
	double c[3];
	double circumcenter[3];

	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		/* a-Vektor-Coordinates (a = P1P2)*/ 
		element_vector[i]->a1 = ((element_vector[i]->x2)-(element_vector[i]->x1));
		element_vector[i]->a2 = ((element_vector[i]->y2)-(element_vector[i]->y1));
		element_vector[i]->a3 = ((element_vector[i]->z2)-(element_vector[i]->z1));
		/* b-Vektor-Coordinates (a = P1P3)*/ 
		element_vector[i]->b1 = ((element_vector[i]->x3)-(element_vector[i]->x1));
		element_vector[i]->b2 = ((element_vector[i]->y3)-(element_vector[i]->y1));
		element_vector[i]->b3 = ((element_vector[i]->z3)-(element_vector[i]->z1));
		/* c-Vektor-Coordinates (a = P3P2)*/ 
		element_vector[i]->c1 = ((element_vector[i]->x2)-(element_vector[i]->x3));
		element_vector[i]->c2 = ((element_vector[i]->y2)-(element_vector[i]->y3));
		element_vector[i]->c3 = ((element_vector[i]->z2)-(element_vector[i]->z3));
	
		/* |a| - von P1 zu P2*/ 
		element_vector[i]->a_quantum = sqrt(
		(((element_vector[i]->x2)-(element_vector[i]->x1))*((element_vector[i]->x2)-(element_vector[i]->x1)))+
		(((element_vector[i]->y2)-(element_vector[i]->y1))*((element_vector[i]->y2)-(element_vector[i]->y1)))+
		(((element_vector[i]->z2)-(element_vector[i]->z1))*((element_vector[i]->z2)-(element_vector[i]->z1))));

        /* |b| - von P1 zu P3*/ 
		element_vector[i]->b_quantum = sqrt(
		(((element_vector[i]->x3)-(element_vector[i]->x1))*((element_vector[i]->x3)-(element_vector[i]->x1)))+
		(((element_vector[i]->y3)-(element_vector[i]->y1))*((element_vector[i]->y3)-(element_vector[i]->y1)))+
		(((element_vector[i]->z3)-(element_vector[i]->z1))*((element_vector[i]->z3)-(element_vector[i]->z1))));

		/* |c| - von P3 zu P2*/ 
		element_vector[i]->c_quantum = sqrt(
		(((element_vector[i]->x2)-(element_vector[i]->x3))*((element_vector[i]->x2)-(element_vector[i]->x3)))+
		(((element_vector[i]->y2)-(element_vector[i]->y3))*((element_vector[i]->y2)-(element_vector[i]->y3)))+
		(((element_vector[i]->z2)-(element_vector[i]->z3))*((element_vector[i]->z2)-(element_vector[i]->z3))));

		/* cos_alpha - between Vektor a&b -> P1P2&P1P3 -> Angle at P1*/ 
		element_vector[i]->cos_alpha = ((element_vector[i]->c_quantum*element_vector[i]->c_quantum)+
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum)-
									   (element_vector[i]->a_quantum*element_vector[i]->a_quantum))
									   /(2*(element_vector[i]->b_quantum*element_vector[i]->c_quantum));

		/* cos_beta - between Vektor a&c -> P1P2&P2P3 -> Angle at P2*/ 
		element_vector[i]->cos_beta = ((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									   (element_vector[i]->c_quantum*element_vector[i]->c_quantum)-
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->c_quantum));

		/* cos_gamma - between Vektor c&b -> P3P2&P1P3 -> Angle at P3*/ 
		element_vector[i]->cos_gamma =((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum)-
									   (element_vector[i]->c_quantum*element_vector[i]->c_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->b_quantum));

		/* alpha_rad*/ 
		element_vector[i]->alpha_rad = acos(element_vector[i]->cos_alpha);
		/* beta_rad*/ 
		element_vector[i]->beta_rad = acos(element_vector[i]->cos_beta);
		/* gamma_rad*/ 
		element_vector[i]->gamma_rad = acos(element_vector[i]->cos_gamma);

		/* alpha_deg*/ 
		element_vector[i]->alpha_deg = ( 180/pi )* acos(element_vector[i]->cos_alpha);
		/* beta_deg*/ 
		element_vector[i]->beta_deg = ( 180/pi )* acos(element_vector[i]->cos_beta);
		/* gamma_deg*/ 
		element_vector[i]->gamma_deg = ( 180/pi )* acos(element_vector[i]->cos_gamma);

		/* Triangle Area*/ 
		check_value =	element_vector[i]->area = ( 0.5 )* 
						          (element_vector[i]->a_quantum*element_vector[i]->b_quantum) *
								  sin(element_vector[i]->gamma_rad);
		/* Triangle Perimeter*/ 
		element_vector[i]->perimeter = (element_vector[i]->a_quantum)+
									   (element_vector[i]->b_quantum)+
								       (element_vector[i]->c_quantum);
		
	    /* Center of Circumcircle/Circumsphere*/ 	
		a[0] = element_vector[i]->x1;
	    a[1] = element_vector[i]->y1;
	    a[2] = element_vector[i]->z1;
	    b[0] = element_vector[i]->x2;
	    b[1] = element_vector[i]->y2;
	    b[2] = element_vector[i]->z2;
        c[0] = element_vector[i]->x3;
	    c[1] = element_vector[i]->y3;
	    c[2] = element_vector[i]->z3;
	    TriCircumcenter3d(a,b,c,circumcenter);
	    element_vector[i]->circumcenter_x = circumcenter[0];
	    element_vector[i]->circumcenter_y = circumcenter[1];
	    element_vector[i]->circumcenter_z = circumcenter[2];

		/* Radius Circumcircle/Circumsphere*/ 
		element_vector[i]->circumsphere_radius =  sqrt(
		(((element_vector[i]->circumcenter_x)-(element_vector[i]->x1))*((element_vector[i]->circumcenter_x)-(element_vector[i]->x1)))+
		(((element_vector[i]->circumcenter_y)-(element_vector[i]->y1))*((element_vector[i]->circumcenter_y)-(element_vector[i]->y1)))+
		(((element_vector[i]->circumcenter_z)-(element_vector[i]->z1))*((element_vector[i]->circumcenter_z)-(element_vector[i]->z1))));

		
	}


}


/**************************************************************************/
/* MshLib - Funktion: CalculateTriangleAreaQuality
                                                                          */
/* Aufgabe: CalculateTriangleAreaQuality
			geht durch das Datenkonstrukt und berechnet ein auf eins normiertes
			Dreicksflächen-Qualitätskriterium
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateTriangleAreaQuality()
{
	long number_of_all_elements = 0;
	int i=0;
	double quality_factor;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		quality_factor=element_vector[i]->area_quality = (4*(sqrt(3.0)))*(element_vector[i]->area)
		/ ((element_vector[i]->a_quantum * element_vector[i]->a_quantum)+
           (element_vector[i]->b_quantum * element_vector[i]->b_quantum)+
		   (element_vector[i]->c_quantum * element_vector[i]->c_quantum));

        element_vector[i]->quality_factor = quality_factor=element_vector[i]->area_quality;
	
	}
}


/**************************************************************************/
/* MshLib - Funktion: CalculateTriangleAngleQuality
                                                                          */
/* Aufgabe: CalculateTriangleAreaQuality
			geht durch das Datenkonstrukt und berechnet ein auf eins normiertes
			Dreicksflächen-Qualitätskriterium
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateTriangleAngleQuality()
{
	long number_of_all_elements = 0;
	int i=0;
	double quality_factor=0.0, min_angle=0.0,max_angle=0.0;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		if((element_vector[i]->alpha_rad)<(element_vector[i]->beta_rad) &&
		   (element_vector[i]->alpha_rad)<(element_vector[i]->gamma_rad))
			min_angle = element_vector[i]->alpha_rad;
		if((element_vector[i]->beta_rad)<(element_vector[i]->alpha_rad) &&
		   (element_vector[i]->beta_rad)<(element_vector[i]->gamma_rad))
			min_angle = element_vector[i]->beta_rad;
		if((element_vector[i]->gamma_rad)<(element_vector[i]->beta_rad) &&
		   (element_vector[i]->gamma_rad)<(element_vector[i]->alpha_rad))
			min_angle = element_vector[i]->gamma_rad;

		if((element_vector[i]->alpha_rad)>(element_vector[i]->beta_rad) &&
		   (element_vector[i]->alpha_rad)>(element_vector[i]->gamma_rad))
			max_angle = element_vector[i]->alpha_rad;
		if((element_vector[i]->beta_rad)>(element_vector[i]->alpha_rad) &&
		   (element_vector[i]->beta_rad)>(element_vector[i]->gamma_rad))
			max_angle = element_vector[i]->beta_rad;
		if((element_vector[i]->gamma_rad)>(element_vector[i]->beta_rad) &&
		   (element_vector[i]->gamma_rad)>(element_vector[i]->alpha_rad))
			max_angle = element_vector[i]->gamma_rad;


		element_vector[i]->angle_quality = quality_factor = min_angle/max_angle;
                element_vector[i]->quality_factor = quality_factor=element_vector[i]->angle_quality;

	
	}
}

/**************************************************************************/
/* MshLib - Funktion: CalculateTriangleLengthQuality
                                                                          */
/* Aufgabe: CalculateTriangleLengthQuality
			geht durch das Datenkonstrukt und berechnet ein auf eins normiertes
			Dreicksflächen-Qualitätskriterium
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateTriangleLengthQuality()
{
	long number_of_all_elements = 0;
	int i=0;
	double quality_factor=0.0, min_length=0.0,max_length=0.0;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		if((element_vector[i]->a_quantum)<(element_vector[i]->b_quantum) &&
		   (element_vector[i]->a_quantum)<(element_vector[i]->c_quantum))
			min_length = element_vector[i]->a_quantum;
		if((element_vector[i]->b_quantum)<(element_vector[i]->a_quantum) &&
		   (element_vector[i]->b_quantum)<(element_vector[i]->c_quantum))
			min_length = element_vector[i]->b_quantum;
		if((element_vector[i]->c_quantum)<(element_vector[i]->a_quantum) &&
		   (element_vector[i]->c_quantum)<(element_vector[i]->b_quantum))
			min_length = element_vector[i]->c_quantum;
		if((element_vector[i]->a_quantum)>(element_vector[i]->b_quantum) &&
		   (element_vector[i]->a_quantum)>(element_vector[i]->c_quantum))
			max_length = element_vector[i]->a_quantum;
		if((element_vector[i]->b_quantum)>(element_vector[i]->a_quantum) &&
		   (element_vector[i]->b_quantum)>(element_vector[i]->c_quantum))
			max_length = element_vector[i]->b_quantum;
		if((element_vector[i]->c_quantum)>(element_vector[i]->a_quantum) &&
		   (element_vector[i]->c_quantum)>(element_vector[i]->b_quantum))
			max_length = element_vector[i]->c_quantum;

		quality_factor=element_vector[i]->length_quality = min_length/max_length;
                element_vector[i]->quality_factor = quality_factor=element_vector[i]->length_quality;
	
	
	}
}



/**************************************************************************/
/* MshLib - Funktion: CalculateBasicTetrahedraData
                                                                          */
/* Aufgabe: CalculateBasicTriangleData l
			geht durch das Datenkonstrukt und berechnet die Vektorlängen Winkel Fläche
			und schreibt die entsprechende Werte in das Konstrukt
			berechnete und geschrieben Werte: |a|&|b|&|c|& alpha & beta & gamma & Fläche A
											  QA (area quality)
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateBasicTetrahedraData()
{
	long number_of_all_elements = 0;
	int i=0;
	double check_value;
	double pi = 3.14159265359;
	double a[3][3];
    double s1,s2,s3,s4,s5,s6;
    //double volume;

	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		/* a-Vektor-Coordinates (a = P1P2)*/ 
		element_vector[i]->a1 = ((element_vector[i]->x2)-(element_vector[i]->x1));
		element_vector[i]->a2 = ((element_vector[i]->y2)-(element_vector[i]->y1));
		element_vector[i]->a3 = ((element_vector[i]->z2)-(element_vector[i]->z1));
		/* b-Vektor-Coordinates (a = P1P3)*/ 
		element_vector[i]->b1 = ((element_vector[i]->x3)-(element_vector[i]->x1));
		element_vector[i]->b2 = ((element_vector[i]->y3)-(element_vector[i]->y1));
		element_vector[i]->b3 = ((element_vector[i]->z3)-(element_vector[i]->z1));
		/* c-Vektor-Coordinates (a = P3P2)*/ 
		element_vector[i]->c1 = ((element_vector[i]->x2)-(element_vector[i]->x3));
		element_vector[i]->c2 = ((element_vector[i]->y2)-(element_vector[i]->y3));
		element_vector[i]->c3 = ((element_vector[i]->z2)-(element_vector[i]->z3));
		/* d-Vektor-Coordinates (a = P1P2)*/ 
		element_vector[i]->d1 = ((element_vector[i]->x4)-(element_vector[i]->x1));
		element_vector[i]->d2 = ((element_vector[i]->y4)-(element_vector[i]->y1));
		element_vector[i]->d3 = ((element_vector[i]->z4)-(element_vector[i]->z1));
		/* e-Vektor-Coordinates (a = P1P3)*/ 
		element_vector[i]->e1 = ((element_vector[i]->x4)-(element_vector[i]->x2));
		element_vector[i]->e2 = ((element_vector[i]->y4)-(element_vector[i]->y2));
		element_vector[i]->e3 = ((element_vector[i]->z4)-(element_vector[i]->z2));
		/* f-Vektor-Coordinates (a = P3P2)*/ 
		element_vector[i]->f1 = ((element_vector[i]->x4)-(element_vector[i]->x3));
		element_vector[i]->f2 = ((element_vector[i]->y4)-(element_vector[i]->y3));
		element_vector[i]->f3 = ((element_vector[i]->z4)-(element_vector[i]->z3));
		double test = 0.0;
		/* |a| - von P1 zu P2*/ 
		test = element_vector[i]->a_quantum = sqrt(
		((element_vector[i]->a1)*(element_vector[i]->a1))+
		((element_vector[i]->a2)*(element_vector[i]->a2))+
		((element_vector[i]->a3)*(element_vector[i]->a3)));
        /* |b| - von P1 zu P3*/ 
		test = element_vector[i]->b_quantum = sqrt(
		((element_vector[i]->b1)*(element_vector[i]->b1))+
		((element_vector[i]->b2)*(element_vector[i]->b2))+
		((element_vector[i]->b3)*(element_vector[i]->b3)));
		/* |c| - von P3 zu P2*/ 
		test = element_vector[i]->c_quantum = sqrt(
		((element_vector[i]->c1)*(element_vector[i]->c1))+
		((element_vector[i]->c2)*(element_vector[i]->c2))+
		((element_vector[i]->c3)*(element_vector[i]->c3)));
		/* |d| - von P4 zu P1*/ 
		test = element_vector[i]->d_quantum = sqrt(
		((element_vector[i]->d1)*(element_vector[i]->d1))+
		((element_vector[i]->d2)*(element_vector[i]->d2))+
		((element_vector[i]->d3)*(element_vector[i]->d3)));
        /* |e| - von P4 zu P2*/ 
		test = element_vector[i]->e_quantum = sqrt(
		((element_vector[i]->e1)*(element_vector[i]->e1))+
		((element_vector[i]->e2)*(element_vector[i]->e2))+
		((element_vector[i]->e3)*(element_vector[i]->e3)));
		/* |f| - von P4 zu P3*/ 
		test = element_vector[i]->f_quantum = sqrt(
		((element_vector[i]->f1)*(element_vector[i]->f1))+
		((element_vector[i]->f2)*(element_vector[i]->f2))+
		((element_vector[i]->f3)*(element_vector[i]->f3)));


		/* Triangle abc */ 
		/* cos_alpha - between Vektor c&b */ 
		test = element_vector[i]->cos_alpha = ((element_vector[i]->c_quantum*element_vector[i]->c_quantum)+
									    (element_vector[i]->b_quantum*element_vector[i]->b_quantum)-
									    (element_vector[i]->a_quantum*element_vector[i]->a_quantum))
									   /(2*(element_vector[i]->b_quantum*element_vector[i]->c_quantum));

        if(element_vector[i]->cos_alpha )
		/* cos_beta - between Vektor a&c  */ 
		test = element_vector[i]->cos_beta = ((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									   (element_vector[i]->c_quantum*element_vector[i]->c_quantum)-
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->c_quantum));

		/* cos_gamma - between Vektor a&b */ 
		test = element_vector[i]->cos_gamma =((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum)-
									   (element_vector[i]->c_quantum*element_vector[i]->c_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->b_quantum));

		/* Triangle aed */ 
		/* cos_alpha - between Vektor a&d */ 
		test = element_vector[i]->cos_alpha1 = ((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									    (element_vector[i]->d_quantum*element_vector[i]->d_quantum)-
									    (element_vector[i]->e_quantum*element_vector[i]->e_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->d_quantum));

		/* cos_beta - between Vektor a&e  */ 
		test = element_vector[i]->cos_beta1 = ((element_vector[i]->a_quantum*element_vector[i]->a_quantum)+
									   (element_vector[i]->e_quantum*element_vector[i]->e_quantum)-
									   (element_vector[i]->d_quantum*element_vector[i]->d_quantum))
									   /(2*(element_vector[i]->a_quantum*element_vector[i]->e_quantum));

		/* cos_gamma - between Vektor e&d */ 
		test = element_vector[i]->cos_gamma1 =((element_vector[i]->e_quantum*element_vector[i]->e_quantum)+
									   (element_vector[i]->d_quantum*element_vector[i]->d_quantum)-
									   (element_vector[i]->a_quantum*element_vector[i]->a_quantum))
									   /(2*(element_vector[i]->e_quantum*element_vector[i]->d_quantum));

		/* Triangle bfd */ 
		/* cos_alpha - between Vektor b&f */ 
		test = element_vector[i]->cos_alpha2 = ((element_vector[i]->b_quantum*element_vector[i]->b_quantum)+
									    (element_vector[i]->f_quantum*element_vector[i]->f_quantum)-
									    (element_vector[i]->d_quantum*element_vector[i]->d_quantum))
									   /(2*(element_vector[i]->b_quantum*element_vector[i]->f_quantum));

		/* cos_beta - between Vektor b&d  */ 
		test = element_vector[i]->cos_beta2 = ((element_vector[i]->b_quantum*element_vector[i]->b_quantum)+
									   (element_vector[i]->d_quantum*element_vector[i]->d_quantum)-
									   (element_vector[i]->f_quantum*element_vector[i]->f_quantum))
									   /(2*(element_vector[i]->b_quantum*element_vector[i]->d_quantum));

		/* cos_gamma - between Vektor f&d */ 
		test = element_vector[i]->cos_gamma2 =((element_vector[i]->f_quantum*element_vector[i]->f_quantum)+
									   (element_vector[i]->d_quantum*element_vector[i]->d_quantum)-
									   (element_vector[i]->b_quantum*element_vector[i]->b_quantum))
									   /(2*(element_vector[i]->f_quantum*element_vector[i]->d_quantum));
		
		/* Triangle cfe */ 
		/* cos_alpha - between Vektor c&e */ 
		test = element_vector[i]->cos_alpha3 = ((element_vector[i]->c_quantum*element_vector[i]->c_quantum)+
									    (element_vector[i]->e_quantum*element_vector[i]->e_quantum)-
									    (element_vector[i]->f_quantum*element_vector[i]->f_quantum))
									   /(2*(element_vector[i]->c_quantum*element_vector[i]->e_quantum));

		/* cos_beta - between Vektor c&f  */ 
		test = element_vector[i]->cos_beta3 = ((element_vector[i]->c_quantum*element_vector[i]->c_quantum)+
									   (element_vector[i]->f_quantum*element_vector[i]->f_quantum)-
									   (element_vector[i]->e_quantum*element_vector[i]->e_quantum))
									   /(2*(element_vector[i]->c_quantum*element_vector[i]->f_quantum));

		/* cos_gamma - between Vektor e&f */ 
		test = element_vector[i]->cos_gamma3 =((element_vector[i]->e_quantum*element_vector[i]->e_quantum)+
									   (element_vector[i]->f_quantum*element_vector[i]->f_quantum)-
									   (element_vector[i]->c_quantum*element_vector[i]->c_quantum))
									   /(2*(element_vector[i]->e_quantum*element_vector[i]->f_quantum));

		/* alpha_rad*/ 
		test = element_vector[i]->alpha_rad = acos(element_vector[i]->cos_alpha);
		test = element_vector[i]->alpha_rad1 = acos(element_vector[i]->cos_alpha1);
		test = element_vector[i]->alpha_rad2 = acos(element_vector[i]->cos_alpha2);
		test = element_vector[i]->alpha_rad3 = acos(element_vector[i]->cos_alpha3);
		/* beta_rad*/ 
		test = element_vector[i]->beta_rad = acos(element_vector[i]->cos_beta);
		test = element_vector[i]->beta_rad1 = acos(element_vector[i]->cos_beta1);
		test = element_vector[i]->beta_rad2 = acos(element_vector[i]->cos_beta2);
		test = element_vector[i]->beta_rad3 = acos(element_vector[i]->cos_beta3);
		/* gamma_rad*/ 
		test = element_vector[i]->gamma_rad = acos(element_vector[i]->cos_gamma);
		test = element_vector[i]->gamma_rad1 = acos(element_vector[i]->cos_gamma1);
		test = element_vector[i]->gamma_rad2 = acos(element_vector[i]->cos_gamma2);
		test = element_vector[i]->gamma_rad3 = acos(element_vector[i]->cos_gamma3);
		/* alpha_deg*/ 
		test = element_vector[i]->alpha_deg = ( 180/pi )* acos(element_vector[i]->cos_alpha);
		test = element_vector[i]->alpha_deg1 = ( 180/pi )* acos(element_vector[i]->cos_alpha1);
		test = element_vector[i]->alpha_deg2 = ( 180/pi )* acos(element_vector[i]->cos_alpha2);
		test = element_vector[i]->alpha_deg3 = ( 180/pi )* acos(element_vector[i]->cos_alpha3);
		/* beta_deg*/ 
		test = element_vector[i]->beta_deg = ( 180/pi )* acos(element_vector[i]->cos_beta);
		test = element_vector[i]->beta_deg1 = ( 180/pi )* acos(element_vector[i]->cos_beta1);
		test = element_vector[i]->beta_deg2 = ( 180/pi )* acos(element_vector[i]->cos_beta2);
		test = element_vector[i]->beta_deg3 = ( 180/pi )* acos(element_vector[i]->cos_beta3);
		/* gamma_deg*/ 
		test = element_vector[i]->gamma_deg = ( 180/pi )* acos(element_vector[i]->cos_gamma);
		test = element_vector[i]->gamma_deg1 = ( 180/pi )* acos(element_vector[i]->cos_gamma1);
		test = element_vector[i]->gamma_deg2 = ( 180/pi )* acos(element_vector[i]->cos_gamma2);
		test = element_vector[i]->gamma_deg3 = ( 180/pi )* acos(element_vector[i]->cos_gamma3);


		/* Terahedra Triangle Perimeter*/ 
		/*TRI: abc*/ 
		element_vector[i]->perimeter_tetra_tri_1 = (element_vector[i]->a_quantum)+
													   (element_vector[i]->b_quantum)+
												       (element_vector[i]->c_quantum);
        /*TRI: bfe*/ 
		element_vector[i]->perimeter_tetra_tri_2 = (element_vector[i]->b_quantum)+
													   (element_vector[i]->f_quantum)+
												       (element_vector[i]->e_quantum);
		/*TRI: aed*/ 
		element_vector[i]->perimeter_tetra_tri_3 = (element_vector[i]->a_quantum)+
													   (element_vector[i]->e_quantum)+
												       (element_vector[i]->d_quantum);
		/*TRI: cdf*/ 
		element_vector[i]->perimeter_tetra_tri_4 = (element_vector[i]->c_quantum)+
													   (element_vector[i]->d_quantum)+
												       (element_vector[i]->f_quantum);


		/* Tetrahedra Triangle Area*/ 
		/*TRI: abc*/	
		element_vector[i]->tetra_tri_area_1 = sqrt(
         (0.5*element_vector[i]->perimeter_tetra_tri_1)*
		((0.5*element_vector[i]->perimeter_tetra_tri_1)-element_vector[i]->a_quantum)*
        ((0.5*element_vector[i]->perimeter_tetra_tri_1)-element_vector[i]->b_quantum)*
		((0.5*element_vector[i]->perimeter_tetra_tri_1)-element_vector[i]->c_quantum));
		/*TRI: bfe*/	
		element_vector[i]->tetra_tri_area_2 = sqrt(
         (0.5*element_vector[i]->perimeter_tetra_tri_2)*
		((0.5*element_vector[i]->perimeter_tetra_tri_2)-element_vector[i]->b_quantum)*
        ((0.5*element_vector[i]->perimeter_tetra_tri_2)-element_vector[i]->f_quantum)*
		((0.5*element_vector[i]->perimeter_tetra_tri_2)-element_vector[i]->e_quantum));
		/*TRI: aed*/ 
		element_vector[i]->tetra_tri_area_3 = sqrt(
         (0.5*element_vector[i]->perimeter_tetra_tri_3)*
		((0.5*element_vector[i]->perimeter_tetra_tri_3)-element_vector[i]->a_quantum)*
        ((0.5*element_vector[i]->perimeter_tetra_tri_3)-element_vector[i]->e_quantum)*
		((0.5*element_vector[i]->perimeter_tetra_tri_3)-element_vector[i]->d_quantum));
		/*TRI: cdf*/ 	
		element_vector[i]->tetra_tri_area_4 = sqrt(
         (0.5*element_vector[i]->perimeter_tetra_tri_4)*
		((0.5*element_vector[i]->perimeter_tetra_tri_4)-element_vector[i]->c_quantum)*
        ((0.5*element_vector[i]->perimeter_tetra_tri_4)-element_vector[i]->d_quantum)*
		((0.5*element_vector[i]->perimeter_tetra_tri_4)-element_vector[i]->f_quantum));

		/* Tetrahedra Area*/ 
		element_vector[i]->tetra_area = 
			((element_vector[i]->tetra_tri_area_1) + 
			(element_vector[i]->tetra_tri_area_2) +
	        (element_vector[i]->tetra_tri_area_3) + 
			(element_vector[i]->tetra_tri_area_4));
		
		/* Tetrahedra Volume*/ 
		//    Beschreibung:  Berechnet das Volumen des Tetraeders.
		//    Rueckgabewert: Volumen
		//    Bemerkung:     Algorithmus: a,b,c,d seien die Ecken des Tetraeders, 
		//                   p = ab, q = ac, r = ad Vektoren.
		//                   Das Volumen ist 1/6 der Determinante der 3x3-Matrix
   		a[0][0] = element_vector[i]->x1 - element_vector[i]->x2;
		a[0][1] = element_vector[i]->y1 - element_vector[i]->y2;
		a[0][2] = element_vector[i]->z1 - element_vector[i]->z2;
   		a[1][0] = element_vector[i]->x1 - element_vector[i]->x3;
		a[1][1] = element_vector[i]->y1 - element_vector[i]->y3;
		a[1][2] = element_vector[i]->z1 - element_vector[i]->z3;
   		a[2][0] = element_vector[i]->x1 - element_vector[i]->x4;
		a[2][1] = element_vector[i]->y1 - element_vector[i]->y4;
		a[2][2] = element_vector[i]->z1 - element_vector[i]->z4;	   
		s1=a[0][0]*a[1][1]*a[2][2];
		s2=a[0][1]*a[1][2]*a[2][0];
		s3=a[0][2]*a[1][0]*a[2][1];
		s4=a[0][2]*a[1][1]*a[2][0];
		s5=a[0][0]*a[2][1]*a[1][2];
		s6=a[2][2]*a[1][0]*a[0][1];
		check_value = element_vector[i]->tetra_volume = fabs((s1+s2+s3-s4-s5-s6)/6.0);


	}
}



/**************************************************************************/
/* MshLib - Funktion: CalculateTetrahedraVolumeQuality();
                                                                          */
/* Aufgabe: CalculateTetrahedraVolumeQuality
			geht durch das Datenkonstrukt und berechnet ein normiertes
			Tetraeder-Qualitätskriterium -> Volumenbezogen
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateTetrahedraVolumeQuality()
{
	long number_of_all_elements = 0;
	int i=0;
	double quality_factor;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		quality_factor=	element_vector[i]->volume_quality = (216*sqrt(3.0)*pow(element_vector[i]->tetra_volume,2))
						/ pow(element_vector[i]->tetra_area,3.0);
                element_vector[i]->quality_factor = quality_factor=element_vector[i]->volume_quality;
	}
}


/**************************************************************************/
/* MshLib - Funktion: CalculateTetrahedraTriangleAngleQuality
                                                                          */
/* Aufgabe: CalculateTetrahedraTriangleAngleQuality
			geht durch das Datenkonstrukt und berechnet zu jedem Dreieck des Tetraeders
			ein normiertes Dreicksflächen-Winkel-Qualitätskriterium und sucht die
			schlechteste Winkelqualität heraus
*/
/* Programmaenderungen:
   10/2003     TK        Erste Version
**************************************************************************/
void CalculateTetrahedraTriangleAngleQuality()
{
	long number_of_all_elements = 0;
	int i=0;
	double quality_factor=0.0, min_angle=0.0,max_angle=0.0;
	vector<CMSHElements*> element_vector;
    //CMSHElements *m_elements = NULL;
	element_vector = MSHGetElementVector();
	
	number_of_all_elements = (int)element_vector.size();

	for (i=0;i<number_of_all_elements;i++)
	{
		if((element_vector[i]->alpha_rad)<=(element_vector[i]->beta_rad) &&
		   (element_vector[i]->alpha_rad)<=(element_vector[i]->gamma_rad))
			min_angle = element_vector[i]->alpha_rad;
		if((element_vector[i]->beta_rad)<=(element_vector[i]->alpha_rad) &&
		   (element_vector[i]->beta_rad)<=(element_vector[i]->gamma_rad))
			min_angle = element_vector[i]->beta_rad;
		if((element_vector[i]->gamma_rad)<(element_vector[i]->beta_rad) &&
		   (element_vector[i]->gamma_rad)<(element_vector[i]->alpha_rad))
			min_angle = element_vector[i]->gamma_rad;

		if((element_vector[i]->alpha_rad)>=(element_vector[i]->beta_rad) &&
		   (element_vector[i]->alpha_rad)>=(element_vector[i]->gamma_rad))
			max_angle = element_vector[i]->alpha_rad;
		if((element_vector[i]->beta_rad)>=(element_vector[i]->alpha_rad) &&
		   (element_vector[i]->beta_rad)>=(element_vector[i]->gamma_rad))
			max_angle = element_vector[i]->beta_rad;
		if((element_vector[i]->gamma_rad)>=(element_vector[i]->beta_rad) &&
		   (element_vector[i]->gamma_rad)>=(element_vector[i]->alpha_rad))
			max_angle = element_vector[i]->gamma_rad;

		quality_factor=element_vector[i]->angle_quality = min_angle/max_angle;

		if((element_vector[i]->alpha_rad1)<=(element_vector[i]->beta_rad1) &&
		   (element_vector[i]->alpha_rad1)<=(element_vector[i]->gamma_rad1))
			min_angle = element_vector[i]->alpha_rad1;
		if((element_vector[i]->beta_rad1)<=(element_vector[i]->alpha_rad1) &&
		   (element_vector[i]->beta_rad1)<=(element_vector[i]->gamma_rad1))
			min_angle = element_vector[i]->beta_rad1;
		if((element_vector[i]->gamma_rad1)<=(element_vector[i]->beta_rad1) &&
		   (element_vector[i]->gamma_rad1)<=(element_vector[i]->alpha_rad1))
			min_angle = element_vector[i]->gamma_rad1;
		if((element_vector[i]->alpha_rad1)>=(element_vector[i]->beta_rad1) &&
		   (element_vector[i]->alpha_rad1)>=(element_vector[i]->gamma_rad1))
			max_angle = element_vector[i]->alpha_rad1;
		if((element_vector[i]->beta_rad1)>=(element_vector[i]->alpha_rad1) &&
		   (element_vector[i]->beta_rad1)>=(element_vector[i]->gamma_rad1))
			max_angle = element_vector[i]->beta_rad1;
		if((element_vector[i]->gamma_rad1)>=(element_vector[i]->beta_rad1) &&
		   (element_vector[i]->gamma_rad1)>=(element_vector[i]->alpha_rad1))
			max_angle = element_vector[i]->gamma_rad1;

		if (quality_factor > min_angle/max_angle)
		quality_factor=element_vector[i]->angle_quality = min_angle/max_angle;

		if((element_vector[i]->alpha_rad2)<=(element_vector[i]->beta_rad2) &&
		   (element_vector[i]->alpha_rad2)<=(element_vector[i]->gamma_rad2))
			min_angle = element_vector[i]->alpha_rad2;
		if((element_vector[i]->beta_rad2)<=(element_vector[i]->alpha_rad2) &&
		   (element_vector[i]->beta_rad2)<=(element_vector[i]->gamma_rad2))
			min_angle = element_vector[i]->beta_rad2;
		if((element_vector[i]->gamma_rad2)<=(element_vector[i]->beta_rad2) &&
		   (element_vector[i]->gamma_rad2)<=(element_vector[i]->alpha_rad2))
			min_angle = element_vector[i]->gamma_rad2;
		if((element_vector[i]->alpha_rad2)>=(element_vector[i]->beta_rad2) &&
		   (element_vector[i]->alpha_rad2)>=(element_vector[i]->gamma_rad2))
			max_angle = element_vector[i]->alpha_rad2;
		if((element_vector[i]->beta_rad2)>=(element_vector[i]->alpha_rad2) &&
		   (element_vector[i]->beta_rad2)>=(element_vector[i]->gamma_rad2))
			max_angle = element_vector[i]->beta_rad2;
		if((element_vector[i]->gamma_rad2)>=(element_vector[i]->beta_rad2) &&
		   (element_vector[i]->gamma_rad2)>=(element_vector[i]->alpha_rad2))
			max_angle = element_vector[i]->gamma_rad2;

		if (quality_factor > min_angle/max_angle)
		quality_factor=element_vector[i]->angle_quality = min_angle/max_angle;

		if((element_vector[i]->alpha_rad3)<=(element_vector[i]->beta_rad3) &&
		   (element_vector[i]->alpha_rad3)<=(element_vector[i]->gamma_rad3))
			min_angle = element_vector[i]->alpha_rad3;
		if((element_vector[i]->beta_rad3)<=(element_vector[i]->alpha_rad3) &&
		   (element_vector[i]->beta_rad3)<=(element_vector[i]->gamma_rad3))
			min_angle = element_vector[i]->beta_rad3;
		if((element_vector[i]->gamma_rad3)<=(element_vector[i]->beta_rad3) &&
		   (element_vector[i]->gamma_rad3)<=(element_vector[i]->alpha_rad3))
			min_angle = element_vector[i]->gamma_rad3;
		if((element_vector[i]->alpha_rad3)>=(element_vector[i]->beta_rad3) &&
		   (element_vector[i]->alpha_rad3)>=(element_vector[i]->gamma_rad3))
			max_angle = element_vector[i]->alpha_rad3;
		if((element_vector[i]->beta_rad3)>=(element_vector[i]->alpha_rad3) &&
		   (element_vector[i]->beta_rad3)>=(element_vector[i]->gamma_rad3))
			max_angle = element_vector[i]->beta_rad3;
		if((element_vector[i]->gamma_rad3)>=(element_vector[i]->beta_rad3) &&
		   (element_vector[i]->gamma_rad3)>=(element_vector[i]->alpha_rad3))
			max_angle = element_vector[i]->gamma_rad3;

		if (quality_factor > min_angle/max_angle)
		quality_factor=element_vector[i]->angle_quality = min_angle/max_angle;

                element_vector[i]->quality_factor = quality_factor=element_vector[i]->angle_quality;
	
	}
}

