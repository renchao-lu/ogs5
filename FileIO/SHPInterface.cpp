/**
 * \file SHPInterface.cpp
 * 25/01/2010 KR Initial implementation
 */

#include "SHPInterface.h"
#include "StringTools.h"
#include "MathTools.h"

using namespace GEOLIB;

bool SHPInterface::readSHPInfo(const std::string &filename, int &shapeType, int &numberOfEntities)
{
	SHPHandle hSHP = SHPOpen(filename.c_str(),"rb");
	if(!hSHP) return false;

	double padfMinBound[4], padfMaxBound[4];

	// The SHPGetInfo() function retrieves various information about shapefile as a whole.
	// The bounds are read from the file header, and may be inaccurate if the file was improperly generated.
	SHPGetInfo( hSHP, &numberOfEntities, &shapeType, padfMinBound, padfMaxBound );

	SHPClose(hSHP);
	return true;
}

void SHPInterface::readSHPFile(const std::string &filename, OGSType choice, std::string listName)
{
	int shapeType, numberOfElements;
	double padfMinBound[4], padfMaxBound[4];

	SHPHandle hSHP = SHPOpen(filename.c_str(),"rb");
	SHPGetInfo( hSHP, &numberOfElements, &shapeType, padfMinBound, padfMaxBound );

	if ( ((shapeType-1)%10 == 0)  &&  (choice==SHPInterface::POINT) )   readPoints(hSHP, numberOfElements, listName);
	if ( ((shapeType-1)%10 == 0)  &&  (choice==SHPInterface::STATION) ) readStations(hSHP, numberOfElements, listName);
	if ( ((shapeType-3)%10 == 0 || (shapeType-5)%10 == 0)  &&  (choice==SHPInterface::POLYLINE) ) readPolylines(hSHP, numberOfElements, listName);
	if ( ((shapeType-3)%10 == 0 || (shapeType-5)%10 == 0)  &&  (choice==SHPInterface::POLYGON) )  readPolygons(hSHP, numberOfElements, listName);
}

void SHPInterface::readPoints(const SHPHandle &hSHP, int numberOfElements, std::string listName)
{
	if (numberOfElements>0)
	{
		std::vector<Point*> *points = new std::vector<Point*>();
		SHPObject *hSHPObject;

		for (int i=0; i<numberOfElements; i++)
		{
			hSHPObject = SHPReadObject(hSHP,i);

			Point* pnt = new Point( *(hSHPObject->padfX), *(hSHPObject->padfY), *(hSHPObject->padfZ) );
			points->push_back(pnt);
		}

		_geoObjects->addPointVec(points, listName);
		SHPDestroyObject(hSHPObject); // de-allocate SHPObject
	}
}

void SHPInterface::readStations(const SHPHandle &hSHP, int numberOfElements, std::string listName)
{
	if (numberOfElements>0)
	{
		std::vector<Point*> *stations = new std::vector<Point*>[numberOfElements];
		SHPObject *hSHPObject;

		for (int i=0; i<numberOfElements; i++)
		{
			hSHPObject = SHPReadObject(hSHP,i);
			Station* stn = Station::createStation( number2str(i), *(hSHPObject->padfX), *(hSHPObject->padfY), *(hSHPObject->padfZ) );
			stations->push_back(stn);
		}

		_geoObjects->addStationVec(stations, listName, GEOLIB::getRandomColor());
		SHPDestroyObject(hSHPObject); // de-allocate SHPObject
	}
}


void SHPInterface::readPolylines(const SHPHandle &hSHP, int numberOfElements, std::string listName)
{
	int nextIdx = -1;
	size_t noOfPoints;
	std::vector<Point*> *points = new std::vector<Point*>();
	std::vector<Polyline*> *lines = new std::vector<Polyline*>();
	SHPObject *hSHPObject;

	// for each polyline)
	for (int i=0; i<numberOfElements; i++)
	{
		hSHPObject = SHPReadObject(hSHP,i);
		noOfPoints = hSHPObject->nVertices;

		Polyline* line = new Polyline(*points);

		// for each point in that polyline
		for (size_t j=0; j<noOfPoints; j++)
		{
			Point* pnt = new Point( *(hSHPObject->padfX+j), *(hSHPObject->padfY+j), *(hSHPObject->padfZ+j) );
			nextIdx=-1;

			// check if point already exists
			for (size_t k=0; k<j; k++)
			{
				if ( (j>0) && (sqrNrm2(pnt) == sqrNrm2( (*points)[k] )) )
				{
					nextIdx=k;
					k=j;
				}
			}
			if (nextIdx<0) 
			{
				points->push_back(pnt);
				nextIdx = points->size() - 1;
			}
			line->addPoint(nextIdx);
		}

		// add polyline to polyline vector
		lines->push_back(line);
	}

	if (numberOfElements>0)
	{
		// add points vector to GEOObjects
		_geoObjects->addPointVec(points, listName);
		// add polyline vector to GEOObjects
		_geoObjects->addPolylineVec(lines, listName);

		SHPDestroyObject(hSHPObject); // de-allocate SHPObject
	}
}

void SHPInterface::readPolygons(const SHPHandle &hSHP, int numberOfElements, std::string listName)
{
	this->readPolylines(hSHP, numberOfElements, listName);

	std::vector<Polyline*> *lines = _geoObjects->getPolylineVec(listName);
	size_t nLines = lines->size();

	if (nLines>0)
	{
		std::vector<Surface*> *surfaces = new std::vector<Surface*>(nLines);
		for (size_t i=0; i<nLines; i++)	
		{
			Surface* sfc = new Surface(*lines);
			if ((*lines)[i]->isClosed())
			{
				sfc->addPolyline(i);
				surfaces->push_back(sfc);
			}
		}
		_geoObjects->addSurfaceVec(surfaces, listName);
	}
}
