/**********************************************************************
Module: Equation of State

Task: This file includes coefficients and functions for calculating the 
thermal properties of liquids and gases in relation to density, pressure 
and temperature.

Programming: Norbert Böttcher
			 Aug 2008
**********************************************************************/

#include "stdafx.h" //MFC

#include <math.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <eos.h>
#include <string>

using namespace std;

/**********************************************************************
A list of coefficients for the equation of state. The function K0 returns 
the specified coefficient. !Important: the parameter indices starts at 1!

Parameters:	
			parameter  - specifies the character of the coefficient:
					     for ideal gas part:
						 0 - a; 1 - theta;
						 for residual part:
						 0 -n; 1 -d; 2 -t; 3 -c; 4 -a; 5 -b; 6 -A; 7 -B; 8 -C;
						 9 -D; 10-alpha; 11-beta; 12-gamma; 13-epsilon;

			index	   - the number of the specified coefficient
			part	   - either the ideal or residual gas part
			fluid	   - specifies the medium itself:
						 0 - CO2;
						 1 - H20; !The coefficients for the residual part of 
								   H2O does not exist yet!
***********************************************************************/
double K0 (int parameter, int index,int part, int fluid)
{
  
	double k [14][56];
	double value;

	switch (fluid){
		case 0: // CO2;
			if (part == 0) // ideal gas part
			{
				k[0][0]=  8.37304456;	k[0][1]=  -3.70454304;	k[0][2]=  2.5;			k[0][3]=  1.99427042; 
				k[0][4]=  0.62105248;	k[0][5]=  0.41195293;	k[0][6]=  1.04028922;	k[0][7]=  0.08327678;

				k[1][0]=  0;		k[1][1]=  0;		k[1][2]=  0;			k[1][3]=  3.15163;	
				k[1][4]=  6.1119;	k[1][5]=  6.77708;	k[1][6]=  11.32384;		k[1][7]=  27.08792;

				value = k[parameter][index-1];

			} else //real gas part
			{
			
			k[0][0]=  3.8856823203161E-01; k[0][1]=  2.9385475942740E+00; k[0][2]= -5.5867188534934E+00; 
			k[0][3]= -7.6753199592477E-01; k[0][4]=  3.1729005580416E-01; k[0][5]=  5.4803315897767E-01; 
			k[0][6]=  1.2279411220335E-01; k[0][7]=  2.1658961543220E+00; k[0][8]=  1.5841735109724E+00; 
			k[0][9]= -2.3132705405503E-01; k[0][10]= 5.8116916431436E-02; k[0][11]=-5.5369137205382E-01;
			k[0][12]= 4.8946615909422E-01; k[0][13]=-2.4275739843501E-02; k[0][14]= 6.2494790501678E-02; 
			k[0][15]=-1.2175860225246E-01; k[0][16]=-3.7055685270086E-01; k[0][17]=-1.6775879700426E-02; 
			k[0][18]=-1.1960736637987E-01; k[0][19]=-4.5619362508778E-02; k[0][20]= 3.5612789270346E-02; 
			k[0][21]=-7.4427727132052E-03; k[0][22]=-1.7395704902432E-03; k[0][23]=-2.1810121289527E-02;
			k[0][24]= 2.4332166559236E-02; k[0][25]=-3.7440133423463E-02; k[0][26]= 1.4338715756878E-01; 
			k[0][27]=-1.3491969083286E-01; k[0][28]=-2.3151225053480E-02; k[0][29]= 1.2363125492901E-02; 
			k[0][30]= 2.1058321972940E-03; k[0][31]=-3.3958519026368E-04; k[0][32]= 5.5993651771592E-03; 
			k[0][33]=-3.0335118055646E-04; k[0][34]=-2.1365488688320E+02; k[0][35]= 2.6641569149272E+04;
			k[0][36]=-2.4027212204557E+04; k[0][37]=-2.8341603423999E+02; k[0][38]= 2.1247284400179E+02; 
			k[0][39]=-6.6642276540751E-01; k[0][40]= 7.2608632349897E-01; k[0][41]= 5.5068668612842E-02;

			k[1][0]=  1;	k[1][1]=  1;	k[1][2]=  1;	k[1][3]=  1;	k[1][4]=  2;	k[1][5]=  2; 
			k[1][6]=  3;	k[1][7]=  1;	k[1][8]=  2;	k[1][9]=  4;	k[1][10]= 5;	k[1][11]= 5; 
			k[1][12]= 5; 	k[1][13]= 6;	k[1][14]= 6;	k[1][15]= 6;	k[1][16]= 1;	k[1][17]= 1; 
			k[1][18]= 4;	k[1][19]= 4;	k[1][20]= 4;	k[1][21]= 7;	k[1][22]= 8;	k[1][23]= 2;
			k[1][24]= 3;	k[1][25]= 3;	k[1][26]= 5;	k[1][27]= 5;	k[1][28]= 6;	k[1][29]= 7; 
			k[1][30]= 8;	k[1][31]= 10; 	k[1][32]= 4;	k[1][33]= 8;	k[1][34]= 2;	k[1][35]= 2; 
			k[1][36]= 2;	k[1][37]= 3;	k[1][38]= 3;

			k[2][0]= 0;		k[2][1]= 0.75;	k[2][2]= 1;		k[2][3]= 2;		k[2][4]= 0.75;	k[2][5]= 2;
			k[2][6]= 0.75;	k[2][7]= 1.5;	k[2][8]= 1.5;	k[2][9]= 2.5;	k[2][10]= 0;	k[2][11]= 1.5; 
			k[2][12]= 2;	k[2][13]= 0;	k[2][14]= 1;	k[2][15]= 2;	k[2][16]= 3;	k[2][17]= 6; 
			k[2][18]= 3;	k[2][19]= 6; 	k[2][20]= 8;	k[2][21]= 6;	k[2][22]= 0;	k[2][23]= 7;
			k[2][24]= 12;	k[2][25]= 16;	k[2][26]= 22; 	k[2][27]= 24;	k[2][28]= 16;	k[2][29]= 24; 
			k[2][30]= 8;	k[2][31]= 2;	k[2][32]= 28;	k[2][33]= 14; 	k[2][34]= 1;	k[2][35]= 0; 
			k[2][36]= 1;	k[2][37]= 3;	k[2][38]= 3;
			
			k[3][7]= 1;		k[3][8]= 1;		k[3][9]= 1;		k[3][10]= 1;	k[3][11]= 1;	k[3][12]= 1; 
			k[3][13]= 1;	k[3][14]= 1;	k[3][15]= 1;	k[3][16]= 2;	k[3][17]= 2;	k[3][18]= 2; 
			k[3][19]= 2;	k[3][20]= 2;	k[3][21]= 2;	k[3][22]= 2;	k[3][23]= 3;	k[3][24]= 3; 
			k[3][25]= 3;	k[3][26]= 4;	k[3][27]= 4;	k[3][28]= 4;	k[3][29]= 4;	k[3][30]= 4;
			k[3][31]= 4;	k[3][32]= 5;	k[3][33]= 6;

			k[4][39]= 3.5;		k[4][40]= 3.5;		k[4][41]= 3;  

			k[5][39]= 0.875;	k[5][40]= 0.925;	k[5][41]= 0.875;

			k[6][39]=0.7;		k[6][40]=0.7;		k[6][41]=0.7; 

			k[7][39]=0.3;		k[7][40]=0.3;		k[7][41]=1;  

			k[8][39]= 10;		k[8][40]= 10;		k[8][41]= 12.5; 

			k[9][39]=275;		k[9][40]=275;		k[9][41]=275;
			
			k[10][34]=25;	k[10][35]=25;		k[10][36]=25;		k[10][37]=15;	k[10][38]=20;

			k[11][34]=325;	k[11][35]=300;		k[11][36]=300;		k[11][37]=275;	k[11][38]=275;
			k[11][39]=0.3;	k[11][40]=0.3;		k[11][41]=0.3; 

			k[12][34]=1.16; k[12][35]=1.19;		k[12][36]=1.19;		k[12][37]=1.25; k[12][38]=1.22;

			k[13][34]=1;	k[13][35]=1;		k[13][36]=1;		k[13][37]=1;	k[13][38]=1;
			
			value = k[parameter][index-1];
			}

			break;

		case 1: // H20;
			if (part == 0) // ideal gas part
			{
			k[0][0]=  -8.32044648201;	k[0][1]=   6.6832105268;	k[0][2]=  3.00632;
			k[0][3]=  0.012436;			k[0][4]=  0.97315;			k[0][5]=  1.27950; 
			k[0][6]=  0.96956;			k[0][7]=  0.24873;

			k[1][0]=  0;				k[1][1]=  0;				k[1][2]=  0; 
			k[1][3]=  1.28728967;		k[1][4]=  3.53734222;		k[1][5]=  7.74073708; 
			k[1][6]=  9.24437796;		k[1][7]=  27.5075105;

			value = k[parameter][index-1];

			} else //real gas part
			{
			k[0][0]=  1.2533547935523E-02;k[0][1]=  7.8957634722828E+00;k[0][2]= -8.7803203303561E+00;
			k[0][3]=  3.1802509345418E-01;k[0][4]= -2.6145533859358E-01;k[0][5]= -7.8199751687981E-03;
			k[0][6]=  8.8089493102134E-03;k[0][7]= -6.6856572307965E-01;k[0][8]=  2.0433810950965E-01;	
			k[0][9]= -6.6212605039687E-05;k[0][10]=-1.9232721156002E-01;k[0][11]=-2.5709043003438E-01;
			k[0][12]= 1.6074868486251E-01;k[0][13]=-4.0092828925807E-02;k[0][14]= 3.9343422603254E-07;
			
			k[0][15]=-7.5941377088144E-06;k[0][16]= 5.6250979351888E-04;k[0][17]=-1.5608652257135E-05;
			k[0][18]= 1.1537996422951E-09;k[0][19]= 3.6582165144204E-07;k[0][20]=-1.3251180074668E-12;
			k[0][21]=-6.2639586912454E-10;k[0][22]=-1.0793600908932E-01;k[0][23]= 1.7611491008752E-02;
			k[0][24]= 2.2132295167546E-01;k[0][25]=-4.0247669763528E-01;k[0][26]= 5.8083399985759E-01;
			k[0][27]= 4.9969146990806E-03;k[0][28]=-3.1358700712549E-02;k[0][29]=-7.4315929710341E-01;
			
			k[0][30]= 4.7807329915480E-01;k[0][31]= 2.0527940895948E-02;k[0][32]=-1.3636435110343E-01;
			k[0][33]= 1.4180634400617E-02;k[0][34]= 8.3326504880713E-03;k[0][35]=-2.9052336009585E-02;
			k[0][36]= 3.8615085574206E-02;k[0][37]=-2.0393486513704E-02;k[0][38]=-1.6554050063734E-03;
			k[0][39]= 1.9955571979541E-03;k[0][40]= 1.5870308324157E-04;k[0][41]=-1.6388568342530E-05;
			k[0][42]= 4.3613615723811E-02;k[0][43]= 3.4994005463765E-02;k[0][44]=-7.6788197844621E-02;
			
			k[0][45]= 2.2446277332006E-02;k[0][46]=-6.2689710414685E-05;k[0][47]=-5.5711118565645E-10;
			k[0][48]=-1.9905718354408E-01;k[0][49]= 3.1777497330738E-01;k[0][50]=-1.1841182425981E-01;
			k[0][51]=-3.1306260323435E+01;k[0][52]= 3.1546140237781E+01;k[0][53]=-2.5213154341695E+03;
			k[0][54]=-1.4874640856724E-01;k[0][55]= 3.1806110878444E-01;

			
			k[1][0]=1;	k[1][1]=1;	k[1][2]=1;	k[1][3]=2;	k[1][4]=2;	k[1][5]=3;	k[1][6]=4;	
			k[1][7]=1;	k[1][8]=1;	k[1][9]=1;	k[1][10]=2;	k[1][11]=2;	k[1][12]=3;	k[1][13]=4;	
			k[1][14]=4;	k[1][15]=5;	k[1][16]=7;	k[1][17]=9;	k[1][18]=10;k[1][19]=11;k[1][20]=13;	
			k[1][21]=15;k[1][22]=1;	k[1][23]=2;	k[1][24]=2;	k[1][25]=2;	k[1][26]=3;	k[1][27]=4;
			k[1][28]=4;	k[1][29]=4;	k[1][30]=5;	k[1][31]=6;	k[1][32]=6;	k[1][33]=7;	k[1][34]=9;
			k[1][35]=9;	k[1][36]=9;	k[1][37]=9;	k[1][38]=9;	k[1][39]=10;k[1][40]=10;k[1][41]=12;
			k[1][42]=3;	k[1][43]=4;	k[1][44]=4;	k[1][45]=5;	k[1][46]=14;k[1][47]=3;	k[1][48]=6;
			k[1][49]=6;	k[1][50]=6;	k[1][51]=3;	k[1][52]=3;	k[1][53]=3;

			k[2][0]=-0.5;		k[2][1]=0.875;		k[2][2]=1;		k[2][3]=0.5;		k[2][4]=0.75;
			k[2][5]=0.375;		k[2][6]=1;			k[2][7]=4;		k[2][8]=6;			k[2][9]=12;
			k[2][10]=1;			k[2][11]=5;			k[2][12]=4;		k[2][13]=2;			k[2][14]=13;
			k[2][15]=9;			k[2][16]=3;			k[2][17]=4;		k[2][18]=11;		k[2][19]=4;
			k[2][20]=13;		k[2][21]=1;			k[2][22]=7;		k[2][23]=1;			k[2][24]=9;
			k[2][25]=10;		k[2][26]=10;		k[2][27]=3;		k[2][28]=7;			k[2][29]=10;
			k[2][30]=10;		k[2][31]=6;			k[2][32]=10;	k[2][33]=10;		k[2][34]=1;
			k[2][35]=2;			k[2][36]=3;			k[2][37]=4;		k[2][38]=8;			k[2][39]=6;
			k[2][40]=9;			k[2][41]=8;			k[2][42]=16;	k[2][43]=22;		k[2][44]=23;
			k[2][45]=23;		k[2][46]=10;		k[2][47]=50;	k[2][48]=44;		k[2][49]=46;
			k[2][50]=50;		k[2][51]=0;			k[2][52]=1;		k[2][53]=4;

			
			k[3][7]=1;	k[3][8]=1;	k[3][9]=1;	k[3][10]=1;	k[3][11]=1;	k[3][12]=1;	k[3][13]=1;	k[3][14]=1;
			k[3][15]=1;	k[3][16]=1;	k[3][17]=1;	k[3][18]=1;	k[3][19]=1;	k[3][20]=1;	k[3][21]=1;	k[3][22]=2;
			k[3][23]=2;	k[3][24]=2;	k[3][25]=2;	k[3][26]=2;	k[3][27]=2;	k[3][28]=2;	k[3][29]=2;	k[3][30]=2;
			k[3][31]=2;	k[3][32]=2;	k[3][33]=2;	k[3][34]=2;	k[3][35]=2;	k[3][36]=2;	k[3][37]=2;	k[3][38]=2;
			k[3][39]=2;	k[3][40]=2;	k[3][41]=2;	k[3][42]=3;	k[3][43]=3;	k[3][44]=3;	k[3][45]=3;	k[3][46]=4;
			k[3][47]=6;	k[3][48]=6;	k[3][49]=6;	k[3][50]=6;		
			
			
			k[4][54] = 3.5;		k[4][55] = 3.5;		
			
			k[5][54] = 0.85;	k[5][55] = 0.95;

			k[6][54] = 0.32;	k[6][55] = 0.32;

			k[7][54] = 0.2;		k[7][55] = 0.2;

			k[8][54] = 28;		k[8][55] = 32;

			k[9][54] = 700;		k[9][55] = 800;

			k[10][51] = 20;		k[10][52] = 20;		k[10][53] = 20;

			k[11][51] = 150;	k[11][52] = 150;	k[11][53] = 250;	k[11][54] = 0.3;	k[11][55] = 0.3;

			k[12][51] = 1.21;	k[12][52] = 1.21;	k[12][53] = 1.25;

			k[13][51] = 1;	k[13][52] = 1;	k[13][53] = 1;

			value = k[parameter][index-1];

			}
			break;
		default: cout << "fluid number " << fluid << "not in list!" << endl;

			break;
}

return value;
}
/***********************************************************************
This function returns the limits for the particular sum terms of the 
derivations of the free Helmholtz energy depending on the used fluid
***********************************************************************/
int limit (int number, int fluid)
{
	switch (fluid)
	{
		case 0: //CO2 
			switch (number)
			{
				case 1: return 7; 
				case 2: return 34;
				case 3: return 39;
				case 4: return 42;
			default : return 0;
			}
		case 1: //H2O 
			switch (number)
			{
				case 1: return 7;
				case 2: return 51;
				case 3: return 54;
				case 4: return 56; 
			default : return 0;
			}
		default : return 0;
	}
}
/**********************************************************************
Function thermal_properties (fluid, critical_density, critical_temperature, specific_gas_constant)
returns the thermal properties of a given fluid
**********************************************************************/
void thermal_properties (int fluid, double &critical_density, double &critical_temperature, double &specific_gas_constant)
{
switch (fluid)
{
case 0: // CO2
	{	critical_density = 467.6;
		critical_temperature = 304.1282;
		specific_gas_constant = 188.9241;
		break;}
case 1: // H2O
	{	critical_density = 322;
		critical_temperature = 647.096;
		specific_gas_constant = 461.51805; 
		break;}
}

}
/**********************************************************************
Some functions and derivations as shown in [Span&Wagner 1994]
***********************************************************************/
double theta_fn (double tau, double A, double delta, double beta)
{
	return (1-tau)+A*pow(pow(delta-1,2),1/(2*beta));
}

double phi_fn (double C, double delta, double D, double tau)
{
	return exp (-C*pow(delta-1,2)-D*pow(tau-1,2));
}

double delta_fn (double theta_fn, double B, double delta, double alpha)
{
	return pow(theta_fn,2)+B*pow(pow(delta-1,2),alpha);
}

double dDELTApowb_ddelta (double b, double delta_fn, double dDELTA_deriv)
{
	return b*pow(delta_fn,b-1)*dDELTA_deriv;
}

double dDELTA_ddelta (double delta, double A, double theta_fn, double beta, double B, double a)
{
	return 	((delta-1)*(A*theta_fn*2/beta*pow(pow((delta-1),2),(1/(2*beta)-1))+2*B*a*pow(pow((delta-1),2),(a-1))));
}

double d2DELTA_ddelta2 (double delta, double dDELTA_deriv, double A, double theta_fn, double beta, double B, double a)
{
	return 1/(delta-1)*dDELTA_deriv+pow((delta-1),2)*
		(4*B*a*(a-1)*pow(pow((delta-1),2),(a-2))+2*pow(A,2)*pow((1/beta),2)*pow(pow(pow((delta-1),2),(1/(2*beta)-1)),2)
		+A*theta_fn*4/beta*(1/(2*beta)-1)*pow(pow((delta-1),2),(1/(2*beta)-2)));
}

double d2DELTApowb_ddelta2 (double b,double delta_fn,double d2DELTA_deriv,double dDELTA_deriv)
{
	return b*(pow(delta_fn,(b-1))*d2DELTA_deriv+(b-1)*pow(delta_fn,(b-2))*pow(dDELTA_deriv,2));
}

double dphi_ddelta (double C,double delta, double phi)
{
	return -2*C*(delta-1)*phi;
}

double dphi_dtau (double D,double tau, double phi_fn)
{
	return -2*D*(tau-1)*phi_fn;
}

double d2phi_dtau2 (double D,double tau, double phi_fn)
{
	return (2*D*pow((tau-1),2)-1)*2*D*phi_fn;
}

double d2phi_ddelta2 (double C,double delta, double phi_fn)
{
	return (2*C*pow((delta-1),2)-1)*2*C*phi_fn;
}

double dDELTA_dtau (double theta_fn, double b, double delta_fn)
{
	return -2*theta_fn*b*pow(delta_fn,(b-1));
}

double d2DELTA_dtau2 (double b, double delta_fn, double theta_fn)
{
	return 2*b*pow(delta_fn,(b-1))+4*pow(theta_fn,2)*b*(b-1)*pow(delta_fn,(b-2));
}

double d2DELTApowb_ddeltadtau (double A,double b,double beta,double delta_fn,double delta,double theta_fn,double dDELTA_deriv)
{
       return -A*b*2/beta*pow(delta_fn,(b-1))*(delta-1)*pow(pow((delta-1),2),(1/(2*beta)-1))
       -2*theta_fn*b*(b-1)*pow(delta_fn,(b-2))*dDELTA_deriv;
}

double d2phi_ddeltadtau (double C,double D,double delta,double tau,double phi_fn)
{
       return 4*C*D*(delta-1)*(tau-1)*phi_fn;
}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_r_d (double rho, double rhoc, double T, double Tc, int fluid)
{
double phi_a=0,phi_b=0,phi_c=0,phi_d=0;
double delta,tau,DELTA,THETA,PHI,DPHI,dDELTA_deriv,dDELTApowbddelta;
int i;

tau = Tc/T;
delta = rho/rhoc;

for (i=1; i<limit(4,fluid)+1; i++)
{
	if (i<limit(1,fluid)+1) 
		
		phi_a=phi_a+(K0(0,i,1,fluid)*K0(1,i,1,fluid)*pow(delta,(K0(1,i,1,fluid)-1))*pow(tau,K0(2,i,1,fluid)));

	else if (i<limit(2,fluid)+1) 
		
		phi_b=phi_b+(K0(0,i,1,fluid)*exp(-pow(delta,K0(3,i,1,fluid)))*(pow(delta,K0(1,i,1,fluid)-1)*
		pow(tau,K0(2,i,1,fluid))*(K0(1,i,1,fluid)-K0(3,i,1,fluid)*pow(delta,K0(3,i,1,fluid)))));

	else if (i<limit(3,fluid)+1) 
		
		phi_c=phi_c+(K0(0,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,K0(2,i,1,fluid))*
		exp(-K0(10,i,1,fluid)*pow(delta-K0(13,i,1,fluid),2)-K0(11,i,1,fluid)*pow(tau-K0(12,i,1,fluid),2))*(K0(1,i,1,fluid)
		/delta-2*K0(10,i,1,fluid)*(delta-K0(13,i,1,fluid))));

	else if (i<limit(4,fluid)+1) {

		THETA  = theta_fn (tau,K0(6,i,1,fluid),delta,K0(11,i,1,fluid));
		DELTA  = delta_fn (THETA,K0(7,i,1,fluid),delta,K0(4,i,1,fluid));
		PHI    = phi_fn (K0(8,i,1,fluid),delta,K0(9,i,1,fluid),tau);
		dDELTA_deriv = dDELTA_ddelta(delta,K0(6,i,1,fluid),THETA,K0(11,i,1,fluid),K0(7,i,1,fluid),K0(4,i,1,fluid));
		dDELTApowbddelta = dDELTApowb_ddelta(K0(5,i,1,fluid),DELTA,dDELTA_deriv);
		DPHI   = dphi_ddelta (K0(8,i,1,fluid),delta,PHI);
		
		phi_d=phi_d+K0(0,i,1,fluid)*(pow(DELTA,K0(5,i,1,fluid))*(PHI+delta*DPHI)+dDELTApowbddelta*delta*PHI);
		}
}
return phi_a+phi_b+phi_c+phi_d;
}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_r_tt (double rho, double rhoc, double T, double Tc,int fluid)
{
double phi_a=0,phi_b=0,phi_c=0,phi_d=0;
double delta,tau,THETA,PHI,DELTA,DDELTA,D2DELTA,DPHI,D2PHI;
int i=0;

tau = Tc/T;
delta = rho/rhoc;


for (i=1; i<limit(4,fluid)+1; i++)
{
	if (i<limit(1,fluid)+1)  

		phi_a = phi_a+ (K0(0,i,1,fluid)*K0(2,i,1,fluid)*(K0(2,i,1,fluid)-1)*pow(delta,K0(1,i,1,fluid))*
						pow(tau,(K0(2,i,1,fluid)-2)));
	
	else if (i<limit(2,fluid)+1)  

		phi_b = phi_b+ (K0(0,i,1,fluid)*K0(2,i,1,fluid)*(K0(2,i,1,fluid)-1)*
						pow(delta,K0(1,i,1,fluid))*pow(tau,(K0(2,i,1,fluid)-2))*exp(-pow(delta,K0(3,i,1,fluid))));
	
	else if (i<limit(3,fluid)+1)  

		phi_c = phi_c+ (K0(0,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,K0(2,i,1,fluid))*
						exp(-K0(10,i,1,fluid)*pow((delta-K0(13,i,1,fluid)),2)-K0(11,i,1,fluid)*pow((tau-K0(12,i,1,fluid)),2))
						*(pow((K0(2,i,1,fluid)/tau-2*K0(11,i,1,fluid)*(tau-K0(12,i,1,fluid))),2)-K0(2,i,1,fluid)
						/pow(tau,2)-2*K0(11,i,1,fluid)));
	
	else if (i<limit(4,fluid)+1) {

		THETA  = theta_fn (tau,K0(6,i,1,fluid),delta,K0(11,i,1,fluid));
		DELTA  = delta_fn (THETA,K0(7,i,1,fluid),delta,K0(4,i,1,fluid));
		PHI    = phi_fn (K0(8,i,1,fluid),delta,K0(9,i,1,fluid),tau);

		D2DELTA = d2DELTA_dtau2 (K0(5,i,1,fluid),DELTA,THETA);
		DDELTA  = dDELTA_dtau (THETA, K0(5,i,1,fluid), DELTA);
        DPHI    = dphi_dtau (K0(9,i,1,fluid),tau,PHI);
		D2PHI   = d2phi_dtau2 (K0(9,i,1,fluid),tau,PHI);
   		
		 phi_d = phi_d + (K0(0,i,1,fluid)*delta*(D2DELTA*PHI+2*DDELTA*DPHI+pow(DELTA,K0(5,i,1,fluid))*D2PHI));
		}
}
return phi_a+phi_b+phi_c+phi_d;
}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_0_t (double T, double Tc, int fluid)
{
double phi_c=0,phi_d=0,phi_e=0;
double tau;
int i;

tau = Tc/T;


phi_c = K0(0,2,0,fluid);
phi_d = K0(0,3,0,fluid)/tau;

for (i=4; i<9; i++) phi_e= phi_e + (K0(0,i,0,fluid)*K0(1,i,0,fluid)*(1/(1-exp(-K0(1,i,0,fluid)*tau))-1));

return phi_c+phi_d+phi_e;
}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_0_tt (double T, double Tc, int fluid)
{
double phi_d=0,phi_e=0;
double tau;
int i;

tau = Tc/T;

phi_d = K0(0,3,0,fluid)/pow(tau,2);

for (i=4; i<9; i++) phi_e=phi_e+(K0(0,i,0,fluid)*pow(K0(1,i,0,fluid),2)*exp(-K0(1,i,0,fluid)*tau)*pow(1-exp(-K0(1,i,0,fluid)*tau),-2));

return 0-phi_d-phi_e;
}
/**********************************************************************
A derivation of the free energy function phi
last change: Nov 2008 NB 4.8.01
***********************************************************************/
double phi_r_t (double rho, double rhoc, double T, double Tc, int fluid)
{
	double phi_a=0,phi_b=0,phi_c=0,phi_d=0,h;
	int i;
	double delta,tau;
	double thetafn,deltafn,ddeltatau,phifn,dphitau;


	tau = Tc/T;
	delta = rho/rhoc;
	
	for (i=1;i<limit(1,fluid)+1;i++)
	{
		phi_a = phi_a + (K0(0,i,1,fluid)*K0(2,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,(K0(2,i,1,fluid)-1)));
	}

	for  (i=limit(1,fluid)+1;i<limit(2,fluid)+1;i++)
	{
		phi_b = phi_b + (K0(0,i,1,fluid)*K0(2,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,(K0(2,i,1,fluid)-1))*exp(-pow(delta,K0(3,i,1,fluid))));
	}
	
	for (i=limit(2,fluid)+1;i<limit(3,fluid)+1;i++)
	{
		phi_c = phi_c + (K0(0,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,(K0(2,i,1,fluid)))*
			exp(-K0(10,i,1,fluid)*pow((delta-K0(13,i,1,fluid)),2)-K0(11,i,1,fluid)*pow((tau-K0(12,i,1,fluid)),2))*
			(K0(2,i,1,fluid)/tau-2*K0(11,i,1,fluid)*(tau-K0(12,i,1,fluid))));
	}

	for (i=limit(3,fluid)+1;i<limit(4,fluid)+1;i++)
	{
		thetafn = theta_fn(tau,K0(6,i,1,fluid),delta,K0(11,i,1,fluid));
		deltafn = delta_fn(thetafn,K0(7,i,1,fluid),delta,K0(4,i,1,fluid));
		ddeltatau = dDELTA_dtau(thetafn,K0(5,i,1,fluid),deltafn);
		phifn = phi_fn(K0(8,i,1,fluid),delta,K0(9,i,1,fluid),tau);
		dphitau = dphi_dtau(K0(9,i,1,fluid),tau,phifn);

		phi_d = phi_d + (K0(0,i,1,fluid)*delta*(ddeltatau*phifn+pow(deltafn,K0(5,i,1,fluid))*dphitau));

	}
    h = phi_a+phi_b+phi_c+phi_d;
	return h;

}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_r_dt (double rho, double rhoc, double T, double Tc, int fluid)
{
	double phi_a=0,phi_b=0,phi_c=0,phi_d=0;
	int i;
	double delta,tau;
    double phifn,thetafn,deltafn,d2phideriv,dDELTAderiv,dDELTApowbddelta,dphidtau,
           dDELTApowbdtau,dphiddelta,d2DELTApowbddeltadtau;
 
		
	tau = Tc/T;
	delta = rho/rhoc;
	
    for (i=1;i<limit(1,fluid)+1;i++)
    {
        phi_a = phi_a + (K0(0,i,1,fluid)*K0(1,i,1,fluid)*K0(2,i,1,fluid)*pow(delta,(K0(1,i,1,fluid)-1))*pow(tau,(K0(2,i,1,fluid)-1)));
	}
    for (i=limit(1,fluid)+1;i<limit(2,fluid)+1;i++)
    {
        phi_b = phi_b + (K0(0,i,1,fluid)*K0(2,i,1,fluid)*pow(delta,(K0(1,i,1,fluid)-1))*pow(tau,(K0(2,i,1,fluid)-1))*(K0(1,i,1,fluid)-K0(3,i,1,fluid)*
        pow(delta,K0(3,i,1,fluid)))*exp(-pow(delta,K0(3,i,1,fluid))));
    }
    for (i=limit(2,fluid)+1;i<limit(3,fluid)+1;i++)
    {
     
	phi_c = phi_c + ((K0(0,i,1,fluid)*pow(delta,K0(1,i,1,fluid))*pow(tau,K0(2,i,1,fluid))*exp(
			-K0(10,i,1,fluid)*pow((delta-K0(13,i,1,fluid)),2)-K0(11,i,1,fluid)*pow((  tau-K0(12,i,1,fluid)),2)))*

			(K0(1,i,1,fluid)/delta-2*K0(10,i,1,fluid)*(delta-K0(13,i,1,fluid)))*
			(K0(2,i,1,fluid)/tau-2*K0(11,i,1,fluid)*(tau-K0(12,i,1,fluid))));
    
	}
    for (i=limit(3,fluid)+1;i<limit(4,fluid)+1;i++)
    {
        phifn = phi_fn(K0(8,i,1,fluid),delta,K0(9,i,1,fluid),tau);
        thetafn = theta_fn(tau,K0(6,i,1,fluid),delta,K0(11,i,1,fluid));
        deltafn = delta_fn(thetafn,K0(7,i,1,fluid),delta,K0(4,i,1,fluid));
        d2phideriv = d2phi_ddeltadtau(K0(8,i,1,fluid),K0(9,i,1,fluid),delta,tau,phifn);
        
		dDELTAderiv = dDELTA_ddelta(delta,K0(6,i,1,fluid),thetafn,K0(11,i,1,fluid),K0(7,i,1,fluid),K0(4,i,1,fluid));
        
		dDELTApowbddelta = dDELTApowb_ddelta(K0(5,i,1,fluid),deltafn,dDELTAderiv);
        dphidtau = dphi_dtau(K0(9,i,1,fluid),tau,phifn);
        dDELTApowbdtau = dDELTA_dtau(thetafn,K0(5,i,1,fluid),deltafn);
        dphiddelta = dphi_ddelta(K0(8,i,1,fluid),delta,phifn);
        d2DELTApowbddeltadtau = d2DELTApowb_ddeltadtau(K0(6,i,1,fluid),K0(5,i,1,fluid),K0(11,i,1,fluid),deltafn,delta,thetafn,dDELTAderiv);
        
        phi_d = phi_d + (K0(0,i,1,fluid)*(pow(deltafn,K0(5,i,1,fluid))*(dphidtau+delta*d2phideriv)+
        delta*dDELTApowbddelta*dphidtau+dDELTApowbdtau*(phifn+delta*dphiddelta)+d2DELTApowbddeltadtau*
        delta*phifn)); }

    return phi_a+phi_b+phi_c+phi_d;
}
/**********************************************************************
A derivation of the free energy function phi
***********************************************************************/
double phi_r_dd (double rho, double rhoc, double T, double Tc, int fluid)
{
	double phi_a=0,phi_b=0,phi_c=0,phi_d=0;
	int i;
	double delta,tau;
	double thetafn,deltafn,phifn,dphiddelta,d2phiddelta2,dDELTA_deriv,dDELTApowbddelta,d2DELTA_deriv,d2DELTApowbddelta2;
		
	tau = Tc/T;
	delta = rho/rhoc;

	for (i=1;i<limit(1,fluid)+1;i++)
	{
		phi_a = phi_a + (K0(0,i,1,fluid)*K0(1,i,1,fluid)*(K0(1,i,1,fluid)-1)*pow(delta,(K0(1,i,1,fluid)-2))*pow(tau,K0(2,i,1,fluid)));
	}
	for (i=limit(1,fluid)+1;i<limit(2,fluid)+1;i++)
	{
		phi_b = phi_b + ((K0(0,i,1,fluid)*exp(-(pow(delta,K0(3,i,1,fluid)))))*
			((pow(delta,(K0(1,i,1,fluid)-2))*pow(tau,K0(2,i,1,fluid)))*
			((K0(1,i,1,fluid)-K0(3,i,1,fluid)*pow(delta,K0(3,i,1,fluid)))*
			(K0(1,i,1,fluid)-1-K0(3,i,1,fluid)*pow(delta,K0(3,i,1,fluid)))-
			(pow(K0(3,i,1,fluid),2)*pow(delta,K0(3,i,1,fluid))))));

	}
	for (i=limit(2,fluid)+1;i<limit(3,fluid)+1;i++)
	{
		phi_c = phi_c + ((K0(0,i,1,fluid)*pow(tau,K0(2,i,1,fluid)))*
						exp(-K0(10,i,1,fluid)*pow((delta-K0(13,i,1,fluid)),2)-K0(11,i,1,fluid)*
						pow((tau-K0(12,i,1,fluid)),2))*(
						(-2*K0(10,i,1,fluid)*pow(delta,K0(1,i,1,fluid))+4*pow(K0(10,i,1,fluid),2)*pow(delta,K0(1,i,1,fluid))*
						pow((delta-K0(13,i,1,fluid)),2))+
						(-4*K0(1,i,1,fluid)*K0(10,i,1,fluid)*pow(delta,(K0(1,i,1,fluid)-1))*(delta-K0(13,i,1,fluid))+
						K0(1,i,1,fluid)*(K0(1,i,1,fluid)-1)*pow(delta,(K0(1,i,1,fluid)-2)))));
	
	}



	for (i=limit(3,fluid)+1;i<limit(4,fluid)+1;i++)
	{
		thetafn        = theta_fn(tau,K0(6,i,1,fluid),delta,K0(11,i,1,fluid));
		deltafn        = delta_fn(thetafn,K0(7,i,1,fluid),delta,K0(4,i,1,fluid));
		phifn          = phi_fn(K0(8,i,1,fluid),delta,K0(9,i,1,fluid),tau);
		dphiddelta      = dphi_ddelta(K0(8,i,1,fluid),delta,phifn);
        d2phiddelta2   = d2phi_ddelta2(K0(8,i,1,fluid),delta,phifn);
		dDELTA_deriv   = dDELTA_ddelta(delta,K0(6,i,1,fluid),thetafn,K0(11,i,1,fluid),K0(7,i,1,fluid),K0(4,i,1,fluid));
		dDELTApowbddelta = dDELTApowb_ddelta(K0(5,i,1,fluid),deltafn,dDELTA_deriv);
		dphiddelta     = dphi_ddelta(K0(8,i,1,fluid),delta,phifn);
		d2DELTA_deriv = d2DELTA_ddelta2(delta,dDELTA_deriv,K0(6,i,1,fluid),thetafn,K0(11,i,1,fluid),K0(7,i,1,fluid),K0(4,i,1,fluid));
		d2DELTApowbddelta2 = d2DELTApowb_ddelta2(K0(5,i,1,fluid),deltafn,d2DELTA_deriv,dDELTA_deriv);


		phi_d = phi_d + (K0(0,i,1,fluid)*(pow(deltafn,K0(5,i,1,fluid))*(2*dphiddelta+delta*d2phiddelta2)
			+2*dDELTApowbddelta*(phifn+delta*dphiddelta)+d2DELTApowbddelta2*delta*phifn));
	}

	return phi_a+phi_b+phi_c+phi_d;
}
/**********************************************************************
Function for calculating the Pressure of a gas/liquid density on density
and temperature. 

Parameters:
			rho  - density
			rhoc - density at the critical point
			T    - temperature
			Tc   - critical temperature
			R    - specific gas constant

Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double pressure (double rho, double T, int fluid)
{
	double P,rhoc,Tc,R;

    thermal_properties (fluid, rhoc, Tc, R);
	P = (1+(rho/rhoc)*phi_r_d(rho,rhoc,T,Tc,fluid))*rho*R*T;

	return P;
}
/**********************************************************************
This function calculates the density depending on pressure and temperature 
by iteration. The iteration process may take a long time, so it's not re-
comended to use this function for simulations. Better use the GetMatrixValue
function and a table with rho-p-T data.
This function does not work for every pressure/temperature Range!

Parameters:
			P    - pressure
			rho0 - initial density for iteration
			rhoc - density at the critical point
			T    - temperature
			Tc   - critical temperature
			R    - specific gas constant
			prec - precision for iteration criteria


Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double density (double P, double rho0, double T, double prec, int fluid)
{
	int iterations=0;
	double rho,p0,rhoc,Tc,R;

	thermal_properties (fluid, rhoc, Tc, R);
    
	p0 = 0;
	while (fabs(P-p0) > prec) {  //change to fabs. 15.09.2008 WW
		rho = P/((1+(rho0/rhoc)*phi_r_d(rho0,rhoc,T,Tc,fluid))*R*T);
		p0  = pressure(rho0,T,fluid);
		rho0 = rho; 
		iterations++;
		if (iterations > 50) return 0;
	}

	return rho;
}
/**********************************************************************
Function for calculating enthalpy depending on density and 
Temperature.

Parameters:
			rho  - density
			rhoc - density at the critical point
			T    - temperature
			Tc   - critical temperature
			R    - specific gas constant


Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double enthalpy (double rho, double T, int fluid)
{
	double h,rhoc,Tc,R;
	double tau, delta;

	thermal_properties (fluid, rhoc, Tc, R);
	tau = Tc/T; 
	delta = rho/rhoc;

	h = (1+tau*(phi_0_t(T,Tc,fluid)+phi_r_t(rho,rhoc,T,Tc,fluid))+delta*phi_r_d(rho,rhoc,T,Tc,fluid))*R*T;

	return h;
}
/**********************************************************************
Function for calculating isochoric heat capacity depending on density and 
Temperature.

Parameters:
			rho  - density
			rhoc - density at the critical point
			T    - temperature
			Tc   - critical temperature
			R    - specific gas constant


Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double isochoric_heat_capacity (double rho, double T, int fluid)
{
	double cv,rhoc,Tc,R;

	thermal_properties (fluid, rhoc, Tc, R);
	cv = -(pow((Tc/T),2)*(phi_0_tt(T,Tc,fluid)+phi_r_tt(rho,rhoc,T,Tc,fluid)))*R;

	return cv;
}
/**********************************************************************
Function for calculating isobaric heat capacity depending on density and 
Temperature.

Parameters:
			rho  - density
			rhoc - density at the critical point
			T    - temperature
			Tc   - critical temperature
			R    - specific gas constant


Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double isobaric_heat_capacity (double rho, double T, int fluid)
{
	double cp,delta,tau,rhoc,Tc,R;
  
 	thermal_properties (fluid, rhoc, Tc, R);
	tau = Tc/T;
	delta = rho/rhoc;

	cp = (-pow(tau,2)*(phi_0_tt(T,Tc,fluid)+phi_r_tt(rho,rhoc,T,Tc,fluid))
		+(pow((1+delta*phi_r_d(rho,rhoc,T,Tc,fluid)-delta*tau*phi_r_dt(rho,rhoc,T,Tc,fluid)),2))
		/((1+2*delta*phi_r_d(rho,rhoc,T,Tc,fluid)+pow(delta,2)*phi_r_dd(rho,rhoc,T,Tc,fluid))))*R;



	return cp;
}
/**********************************************************************
Function for calculating viscosity of CO2 depending on density and Temperature.
Programming: Norbert Böttcher
			 Aug 2008
***********************************************************************/
double co2_viscosity (double rho, double T)
{
  double eta, eta_0, d_eta;
  double psi,t_r;
  double a[5],b[8],d[8][5];
    int i,j;
  

// coefficients of the representation of the zero-density viscosity of co2
  a[0] =  0.235156;
  a[1] = -0.491266;
  a[2] =  0.05211155;
  a[3] =  0.05347906;
  a[4] = -0.01537102;

  psi = 0; // variable for reduced effective cross-section
 

// coefficients of the representation of the excess viscosity of co2
 
  for (i=0; i<8; i++)
  {  for (j=0; j<5; j++) 
  {  b[i] = 0;
   d[i][j] = 0; } }

  d[0][0] = 0.4071119e-02;
  d[1][0] = 0.7198037e-04;
  d[5][3] = 0.2411697e-16;
  d[7][0] = 0.2971072e-22;
  d[7][1] =-0.1627888e-22;


  t_r = T/251.196; //reduced temperature

  // deriving the zero-density viscosity eta_0(T)
 
  for (i=0; i<5; i++)
  {
	  psi = psi + a[i] * pow(log (t_r),(i));
  }
	psi = exp (psi);

	eta_0 = 1.00697 * pow (T,0.5) / psi;
    d_eta = 0;


  // deriving the excess viscosity d_eta(rho,T)
  
  for (i=0; i<8; i++)
  {  for (j=0; j<5; j++) 
	 { 
		 b[i] = b[i] + d[i][j]/pow(t_r,j);
  }
  d_eta = d_eta + b[i] * pow(rho,i+1);
  }


  // deriving dynamic viscosity as sum of eta_o(T) and d_eta(rho,T)
 
  eta = (eta_0+d_eta)* 1e-06; // eta in [Pa s]

  return eta;
}
/**********************************************************************
Function for calculating heat conductivity of CO2 depending on density and Temperature.
	(Vesovic&Wakeham)
Programming: Norbert Böttcher 4.8.01
			 Nov 2008
***********************************************************************/
double co2_heat_conductivity (double rho, double T)
{
double b[8],c[6],d[5];
double G_fn=0,T_r,r,c_int_k,sum_c=0;
int i;
double lamda_0,delta_lamda=0,lamda;

b[0]=0.4226159;
b[1]=0.6280115;
b[2]=-0.5387661;
b[3]=0.6735941;
b[4]=0;
b[5]=0;
b[6]=-0.4362677;
b[7]=0.2255338;

c[1]=0.02387869;
c[2]=4.35079400;
c[3]=-10.33404000;
c[4]=7.98159000;
c[5]=-1.94055800;

d[1]=2.4471640E-02;
d[2]=8.7056050E-05;
d[3]=-6.5479500E-08;
d[4]=6.5949190E-11;


for (i=1;i<6;i++)
    {
    sum_c = sum_c + c[i]*pow((T/100),(2-i));
    }

c_int_k = (1 + exp(-183.5/T))*sum_c;

r = pow((2*c_int_k/5),(0.5));

T_r = T/251.196;

for (i=0;i<8;i++) 
    {
    G_fn = G_fn + (b[i]/pow(T_r,i));
    }

r = 

lamda_0 = (0.475598*pow(T,0.5)*(1+pow(r,2)))/G_fn;

for (i=1;i<5;i++)
{
delta_lamda = delta_lamda + d[i]*pow(rho,i);
}

lamda = (lamda_0 + delta_lamda)/1000;

return lamda;
}

/**********************************************************************
Function for calculating viscosity of CH4 at 295K depending on pressure.
	(Gulik,Mostert,Van den Berg)
Programming: Norbert Böttcher 4.8.01
			 Nov 2008
***********************************************************************/
double ch4_viscosity_295K (double p)
{
double h;

p=p/100000;
h=(-3.7091411E-14*pow(p,4)+9.1937114E-10*pow(p,3)-6.6099446E-06*pow(p,2)+4.8400147E-02*p+1.0934694E+01)/1.0e6;

return h;
}


/**********************************************************************
Viscosity for different Fluids

Programming: Norbert Böttcher 4.8.01
			 Nov 2008
***********************************************************************/
double Fluid_Viscosity (double rho, double T, double p, string caption)
{
double h;
char c;

c = caption[0];

switch (c) {
case 'C' :  // CARBON_DIOXIDE
            h = co2_viscosity (rho,T);
            break;
case 'M' :  // METHANE
            h = ch4_viscosity_295K(p);
            break;
case 'W' :  // WATER
            h = 1E-3;
            break;                        
default :   h = 1E-3;}


//if(caption.find("CARBON_DIOXIDE")!=string::npos)
//      h = co2_viscosity (rho,T);
//if(caption.find("METHANE")!=string::npos)      
//      h = ch4_viscosity_295K(p);
//if(caption.find("WATER")!=string::npos)         
//      h = 1E-3;

return h;}

/**********************************************************************
Heat conductivity for different Fluids

Programming: Norbert Böttcher 4.8.01
			 Nov 2008
***********************************************************************/
double Fluid_Heat_Conductivity (double rho, double T, string caption)
{
double h;
char c;

c = caption[0];

switch (c) {
case 'C' :  // CARBON_DIOXIDE
            h = co2_heat_conductivity (rho,T);
            break;
case 'M' :  // METHANE
            h = 0.0338; // [W/m/K] at 298K and 1 bar
            break;
case 'W' :  // WATER
            h = 0.598; // [W/m/K] at 293K and 1 bar
            break;                        
default :   h = 0.5;}


//if(caption.find("CARBON_DIOXIDE")!=string::npos)
//     
//if(caption.find("METHANE")!=string::npos)      
//      h = 0.0338; // [W/m/K] at 298K and 1 bar
//if(caption.find("WATER")!=string::npos)         
//      h = 0.598; // [W/m/K] at 293K and 1 bar

return h;}
