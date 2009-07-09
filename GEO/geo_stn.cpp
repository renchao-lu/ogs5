/**************************************************************************
GeoLib - Object: Observation Station
Task: 
Programing:
07/2009 KR
**************************************************************************/
// GeoLib
#include "geo_stn.h"

/*----------------------------------------------------------------------*/


// constructor
CGLStation::CGLPoint(void)
{
  m_color[0] = 255;
  m_color[1] = 0;
  m_color[2] = 0;
  name = "STATION";
  string pnt_number;
  pnt_id_last = GEOPointID();
  char pnt_number_char[10];
  sprintf(pnt_number_char,"%i",pnt_id_last);
  name += pnt_number_char;
  //......................................................................
}

// deconstructor
CGLStation::~CGLPoint(void)
{
}
