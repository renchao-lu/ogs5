/*
 * DateTools.cpp
 *
 *  Created on: Jun 16, 2010
 *      Author: KR Initial implementation (in header file)
 *      TF moved implementation to source file
 */

#include "DateTools.h"
#include <cmath>
#include <cstdlib>

double date2Double(int y, int m, int d)
{
	if ( (y<1000 || y>9999) || (m<1 || m>12) || (d<1 || d>31) )
	{
		std::cout << "Error: date2Double() -- input not in expected format.";
		return 0;
	}

	double ddate=0;
	if (y<1900) y+=1900;
	ddate = static_cast<double>(y)*10000;
	ddate += (m*100);
	ddate += d;

	return ddate;
}

std::string date2String(double ddate)
{
	if (ddate<10000101 || ddate>99991231)
	{
		std::cout << "Error: strDate2Double() -- input not in expected format.";
		return "0.0.0000";
	}

	int rest = static_cast<int>(ddate);
	int y = static_cast<int>(floor(rest/10000.0));
	rest = rest % (y*10000);
	int m = static_cast<int>(floor(rest/100.0));
	if (m<1 || m>12) std::cout << "Warning: strDate2Double() -- month not in [1:12]";
	rest = rest % (m*100);
	int d = rest;
	if (d<1 || d>31) std::cout << "Warning: strDate2Double() -- day not in [1:31]";

	std::string day = number2str(d);
	if (d<10) day = "0" + day;
	std::string month = number2str(m);
	if (m<10) month = "0" + month;
	std::string s =  day + "." + month + "." + number2str(y);
	return s;
}

double strDate2Double(const std::string &s)
{
	if (s.length() == 10)
	{
		int d = atoi(s.substr(0,2).c_str());
		if (d<1 || d>31) std::cout << "Warning: strDate2Double() -- day not in [1:31]";
		int m = atoi(s.substr(3,2).c_str());
		if (m<1 || m>12) std::cout << "Warning: strDate2Double() -- month not in [1:12]";
		int y = atoi(s.substr(6,4).c_str());
		return date2Double(y, m, d);
	}
	//std::cout << "Error: strDate2Double() -- input not in expected format.";
	return 0;
}

