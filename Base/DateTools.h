/**
 * \file DateTools.h
 * 22/01/2010 KR Initial implementation
 */

#ifndef DATETOOLS_H
#define DATETOOLS_H

#include "StringTools.h"
#include <string>

/**
 * Converts three integers representing a date into a double.
 * Note: It is not really checked if the date actually makes sense.
 */
double date2Double(int y, int m, int d);

/**
 * Converts a double representing a date into a string.
 * Note: It is not really checked if the date actually makes sense.
 * \param d Number containing date in double format yyyymmdd
 * \return A string containing the date in format "dd.mm.yyyy".
 */
std::string date2String(double ddate);

/**
 * Converts a string containing a date into a double.
 * Note: It is not really checked if the date actually makes sense.
 * \param s String containing the date, the expected format is "dd.mm.yyyy".
 * \return A number representing the date as yyyymmdd.
 */
double strDate2Double(const std::string &s);

#endif //DATETOOLS_H
