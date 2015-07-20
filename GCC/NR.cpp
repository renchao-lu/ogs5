#include <iostream>
#include <math.h>
#include <cmath>
#include <limits>
#include <stdio.h>
#include <string>
#include "NR.h"
using namespace std;


NR::NR(void){}
NR::~NR(void){}


inline double NR::SIGN(const double &a, const double &b)
	{return (double)(b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a));}




double dfridr(double func(double), const double x/*, const double h, double &err*/)
{
	double h=1.0e-3, err;

	const int ntab=10;
	const double con=1.4, con2=(con*con);
	const double big=numeric_limits<double>::max();
	const double safe=2.0;
	int i,j;
	double errt,fac,hh,ans=0.0;
	double a[ntab][ntab];
	if (h == 0.0) throw("h must be nonzero in dfridr.");
	hh=h;
	a[0][0]=(func(x+hh)-func(x-hh))/(2.0*hh);
	err=big;
	for (i=1;i<ntab;i++) {
		hh /= con;
		a[0][i]=(func(x+hh)-func(x-hh))/(2.0*hh);
		fac=con2;
		for (j=1;j<=i;j++) {
			a[j][i]=(a[j-1][i]*fac-a[j-1][i-1])/(fac-1.0);
			fac=con2*fac;
			if(abs(a[j][i]-a[j-1][i])>abs(a[j][i]-a[j-1][i-1]))
				errt=abs(a[j][i]-a[j-1][i]);
			else
				errt=abs(a[j][i]-a[j-1][i-1]);
			if (errt <= err) {
				err=errt;
				ans=a[j][i];
			}
		}
		if (abs(a[i][i]-a[i-1][i-1]) >= safe*err) break;
	}
	return ans;
}

double NR::dfridrX(double func(double,double), const double x, const double y/*, const double h, double &err*/)
{
	double h=1.0e-3, err;

	const int ntab=10;
	const double con=1.4, con2=(con*con);
	const double big=numeric_limits<double>::max();
	const double safe=2.0;
	int i,j;
	double errt,fac,hh,ans=0.0;
	double a[ntab][ntab];
	if (h == 0.0) throw("h must be nonzero in dfridr.");
	hh=h;
	a[0][0]=(func(x+hh,y)-func(x-hh,y))/(2.0*hh);
	err=big;
	for (i=1;i<ntab;i++) {
		hh /= con;
		a[0][i]=(func(x+hh,y)-func(x-hh,y))/(2.0*hh);
		fac=con2;
		for (j=1;j<=i;j++) {
			a[j][i]=(a[j-1][i]*fac-a[j-1][i-1])/(fac-1.0);
			fac=con2*fac;
			if(abs(a[j][i]-a[j-1][i])>abs(a[j][i]-a[j-1][i-1]))
				errt=abs(a[j][i]-a[j-1][i]);
			else
				errt=abs(a[j][i]-a[j-1][i-1]);
			if (errt <= err) {
				err=errt;
				ans=a[j][i];
			}
		}
		if (abs(a[i][i]-a[i-1][i-1]) >= safe*err) break;
	}
	return ans;
}

double NR::dfridrY(double func(double,double), const double x, const double y/*, const double h, double &err*/)
{
	double h=1.0e-3, err;

	const int ntab=10;
	const double con=1.4, con2=(con*con);
	const double big=numeric_limits<double>::max();
	const double safe=2.0;
	int i,j;
	double errt,fac,hh,ans=0.0;
	double a[ntab][ntab];
	if (h == 0.0) throw("h must be nonzero in dfridr.");
	hh=h;
	a[0][0]=(func(x,y+hh)-func(x,y-hh))/(2.0*hh);
	err=big;
	for (i=1;i<ntab;i++) {
		hh /= con;
		a[0][i]=(func(x,y+hh)-func(x,y-hh))/(2.0*hh);
		fac=con2;
		for (j=1;j<=i;j++) {
			a[j][i]=(a[j-1][i]*fac-a[j-1][i-1])/(fac-1.0);
			fac=con2*fac;
			if(abs(a[j][i]-a[j-1][i])>abs(a[j][i]-a[j-1][i-1]))
				errt=abs(a[j][i]-a[j-1][i]);
			else
				errt=abs(a[j][i]-a[j-1][i-1]);
			if (errt <= err) {
				err=errt;
				ans=a[j][i];
			}
		}
		if (abs(a[i][i]-a[i-1][i-1]) >= safe*err) break;
	}
	return ans;
}
