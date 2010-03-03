/***************************************************************************
 *  Point.h
 *  Created on: Feb 4, 2010
 *      Author: KR
**************************************************************************/

#ifndef COLOR_H_
#define COLOR_H_

#include "TemplatePoint.h"

#include <cstdlib>

namespace GEOLIB {
typedef TemplatePoint<unsigned char> Color;

/// Returns a random RGB colour.
static Color* getRandomColor()
{ 
	return new Color((rand()%5)*50, (rand()%5)*50, (rand()%5)*50); 
}

} // namespace

#endif /* COLOR_H_ */
