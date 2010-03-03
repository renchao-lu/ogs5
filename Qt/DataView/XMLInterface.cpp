/**
 * \file XMLInterface.cpp
 * 18/02/2010 KR Initial implementation
 */
#include "XMLInterface.h"

#include <iostream>
#include <fstream>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QXmlStreamAttributes>
#if OGS_QT_VERSION > 45
	#include <QtXmlPatterns/QXmlSchemaValidator>
#endif // QT_VERSION > 45


XMLInterface::XMLInterface(GEOLIB::GEOObjects* geoObjects, std::string schemaFile) : _geoObjects(geoObjects)
{
#if OGS_QT_VERSION > 45
	this->setSchema(schemaFile);
#endif // QT_VERSION > 45
}

int XMLInterface::isValid(const QString &fileName) const
{
#if OGS_QT_VERSION > 45
    if ( _schema.isValid() )
	{
        QXmlSchemaValidator validator( _schema );
		if ( validator.validate( QUrl(fileName) ) )
			return 1;
		else
		{
			std::cout << "XMLInterface::isValid() - XML File is invalid (in reference to the set schema)." << std::endl;
			return 0;
        }
    } else {
        std::cout << "XMLInterface::isValid() - Schema is invalid." << std::endl;
		return -1;
    }
#else
	return 1;
#endif // QT_VERSION > 45
}

#if OGS_QT_VERSION > 45
int XMLInterface::setSchema(const std::string &schemaFile)
{
	_schema.load( QUrl(QString::fromStdString(schemaFile)) );
	if ( _schema.isValid() )
		return 1;
	else
	{
		std::cout << "XMLInterface::setSchema() - Schema is invalid." << std::endl;
		return 0;
	}
}
#endif // QT_VERSION > 45

int XMLInterface::readGLIFile(const QString &fileName)
{
	/*
	if (_schema )
	{
		std::cout << "XMLInterface::readGLIFile() - No schema set." << std::endl;
		return 0;
	}
	*/
	QFile* file = new QFile(fileName);
	QString gliName = "";

	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		std::cout << "XMLInterface::readGLIFile() - Can't open xml-file." << std::endl;
		return 0;
	}

	if (!this->isValid(fileName)) return 0;

	QXmlStreamReader xml(file);
	std::vector<GEOLIB::Point*>    *points    = new std::vector<GEOLIB::Point*>;
	std::vector<GEOLIB::Polyline*> *polylines = new std::vector<GEOLIB::Polyline*>;
	std::vector<GEOLIB::Surface*>  *surfaces  = new std::vector<GEOLIB::Surface*>;

	while(!xml.atEnd() && !xml.hasError())
	{
		// read next element
		QXmlStreamReader::TokenType token = xml.readNext();

		if (token == QXmlStreamReader::StartDocument) continue;

		if (token == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "name")
			{
				xml.readNext();
				gliName = xml.text().toString();
			}
			if (xml.name() == "points")
				this->readPoints(xml, points);
			if (xml.name() == "polylines")
				this->readPolylines(xml, polylines, points);
			if (xml.name() == "surfaces")
				this->readSurfaces(xml, surfaces, polylines);
		}
	}

	// if there are any open xml errors display them
	if(xml.hasError())
	{
		std::cout << "XMLInterface::readGLIFile() - Error: " << xml.errorString().toStdString() << std::endl;
		xml.clear();
		delete file;
		return 0;
	}

	// if no name was given in the xml-file simply use the file name
	if (gliName.isEmpty())
	{
		QFileInfo fi(fileName);
		gliName = fi.baseName();
	}

	std::string gliFinalName = gliName.toStdString();
	_geoObjects->addPointVec(points, gliFinalName);
	_geoObjects->addPolylineVec(polylines, gliFinalName);
	_geoObjects->addSurfaceVec(surfaces, gliFinalName);

	xml.clear();
	delete file;
	return 1;
}

void XMLInterface::readPoints( QXmlStreamReader &xml, std::vector<GEOLIB::Point*> *points )
{
	int x=0, y=0, z=0;
	char* pEnd;
	QXmlStreamAttributes att;
	// check if the current element is indeed the beginning of the points list
	if(xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "points") return;

	xml.readNext();
	// as long as the list of points does not end
	while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "points")) {

		if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "point")
		{
			att = xml.attributes();
			if (att.hasAttribute("id") && att.hasAttribute("x") && att.hasAttribute("y") && att.hasAttribute("z"))
			{
				_idx_map.insert (std::pair<size_t,size_t>(strtol((att.value("id")).toString().toStdString().c_str(), &pEnd, 10), points->size()));
				GEOLIB::Point* p = new GEOLIB::Point(strtod((att.value("x")).toString().toStdString().c_str(), 0),
													 strtod((att.value("y")).toString().toStdString().c_str(), 0),
													 strtod((att.value("z")).toString().toStdString().c_str(), 0));
				points->push_back(p);
			}
			else std::cout << "XMLInterface::readPoints() - Attribute missing in point tag ..." << std::endl;
		}
		xml.readNext();
	}
}

void XMLInterface::readPolylines( QXmlStreamReader &xml, std::vector<GEOLIB::Polyline*> *polylines, std::vector<GEOLIB::Point*> *points )
{
	size_t idx=0;
	QString value = NULL;
	char* pEnd;
	std::map<size_t, size_t> ply_idx;
	QXmlStreamAttributes att;

	if(xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "polylines") return;

	// while within "polylines" section
	while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "polylines"))
	{
		xml.readNext();

		// definition of a new polyline begins
		if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "polyline")
		{
			att = xml.attributes();
			if (att.hasAttribute("id"))
			{
				idx = polylines->size();
				ply_idx.insert (std::pair<size_t,size_t>(strtol((att.value("id")).toString().toStdString().c_str(), &pEnd, 10), idx));
				polylines->push_back(new GEOLIB::Polyline(*points));

				// add points to polyline (as long as polyline does not end)
				while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "polyline"))
				{
					xml.readNext();
					value = readElement(xml, "pnt");
					if (!value.isNull())
						(*polylines)[idx]->addPoint(_idx_map[strtol((value.toStdString()).c_str(), &pEnd, 10)]);
				}
			}
			else std::cout << "XMLInterface::readPolylines() - Attribute missing in polyline tag ..." << std::endl;
		}
	}
	_idx_map = ply_idx; // point index mapping is not needed anymore but polyline index mapping will be needed for creating surfaces
}

void XMLInterface::readSurfaces( QXmlStreamReader &xml, std::vector<GEOLIB::Surface*> *surfaces, std::vector<GEOLIB::Polyline*> *polylines )
{
	size_t idx=0;
	QString value = NULL;
	char* pEnd;
	QXmlStreamAttributes att;

	if(xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "surfaces") return;

	// while within "surfaces" section
	while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "surfaces"))
	{
		xml.readNext();
		// definition of a new surface begins
		if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "surface")
		{
			att = xml.attributes();
			if (att.hasAttribute("id")  &&  att.hasAttribute("matgroup") && att.hasAttribute("epsilon"))
			{
				idx = surfaces->size();
				surfaces->push_back(new GEOLIB::Surface(*polylines));

				// add polylines to surface (as long as surface is not at an end)
				while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "surface"))
				{
					xml.readNext();
					value = readElement(xml, "ply");
					if (!value.isNull())
						(*surfaces)[idx]->addPolyline(_idx_map[strtol((value.toStdString()).c_str(), &pEnd, 10)]);
				}
			}
			else std::cout << "XMLInterface::readSurfaces() - Attribute missing in surface tag ..." << std::endl;
		}
	}
}

QString XMLInterface::readElement(QXmlStreamReader &xml, const QString &name) const
{
	QString value = NULL;
	// until the next EndElement
	while(xml.tokenType() != QXmlStreamReader::EndElement)
	{
		xml.readNext();

		// check if the next StartElement has the right name (if not exit)
		if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == name)
		{
			xml.readNext();
			// has the element a value?
			if(xml.tokenType() != QXmlStreamReader::Characters) return NULL;;
			value = xml.text().toString();
		}
	}
	return value;
}


void XMLInterface::writeGLIFile(QFile* file, const QString &gliName)
{
	size_t nPoints=0, nPolylines=0, nSurfaces=0;

	QXmlStreamWriter xml(file);
	xml.setAutoFormatting(true);
	xml.setCodec(QTextCodec::codecForName("ISO-8859-1"));

	xml.writeStartDocument();

	// to-do: insert stylesheet tag
	// <?xml-stylesheet type="text/xsl" href="OpenGeoSysGLI.xsl"?>
	// reserves space at the location where the style-file entry will be places later
	xml.writeCharacters("                                                                                \n\n\n");
	xml.writeStartElement("OpenGeoSysGLI");
	xml.writeNamespace("http://www.opengeosys.net", "ogs");
	xml.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	xml.writeAttribute("xsi:noNamespaceSchemaLocation", "http://141.65.34.25/OpenGeoSysGLI.xsd");

	xml.writeTextElement("name", gliName);

	// POINTS
	xml.writeStartElement("points");
	std::vector<GEOLIB::Point*> *points = _geoObjects->getPointVec(gliName.toStdString());
	nPoints = points->size();
	for (size_t i=0; i<nPoints; i++)
	{
		xml.writeStartElement("point");
		xml.writeAttribute("id", QString::number(i));
		xml.writeAttribute("x", QString::number((*(*points)[i])[0], 'f'));
		xml.writeAttribute("y", QString::number((*(*points)[i])[1], 'f'));
		xml.writeAttribute("z", QString::number((*(*points)[i])[2], 'f'));
		xml.writeEndElement(); //point
	}
	xml.writeEndElement(); //points


	// POLYLINES
	xml.writeStartElement("polylines");
	std::vector<GEOLIB::Polyline*> *polylines = _geoObjects->getPolylineVec(gliName.toStdString());
	nPolylines = polylines->size();
	for (size_t i=0; i<nPolylines; i++)
	{
		xml.writeStartElement("polyline");
		xml.writeAttribute("id", QString::number(i));

		nPoints = (*polylines)[i]->getSize();
		for (size_t j=0; j<nPoints; j++)
		{
			xml.writeTextElement("pnt", QString::number(((*polylines)[i])->getPointID(j)));
		}
		xml.writeEndElement(); //polyline
	}
	xml.writeEndElement(); //polylines

	// SURFACES
	xml.writeStartElement("surfaces");
	std::vector<GEOLIB::Surface*> *surfaces = _geoObjects->getSurfaceVec(gliName.toStdString());
	nSurfaces = surfaces->size();
	for (size_t i=0; i<nSurfaces; i++)
	{
		xml.writeStartElement("surface");
		xml.writeAttribute("id", QString::number(i));
		xml.writeAttribute("matgroup", QString::number(-1));
		xml.writeAttribute("epsilon", QString::number(0.01));

		nPolylines = (*surfaces)[i]->getSize();
		for (size_t j=0; j<nPolylines; j++)
		{
			xml.writeTextElement("ply", QString::number((*surfaces)[i]->getPolylineID(j)));
		}
		xml.writeEndElement(); //surface
	}
	xml.writeEndElement(); //surfaces

	xml.writeEndElement(); // OpenGeoSysGLI

	xml.writeEndDocument();
}

int XMLInterface::insertStyleFileDefinition(const QString &fileName)
{
	std::string path = fileName.toStdString();
	std::fstream stream(path.c_str());
	std::string line;
	std::string styleDef("\n<?xml-stylesheet type=\"text/xsl\" href=\"OpenGeoSysGLI.xsl\"?>");

	if (!stream.is_open())
    {
		std::cout << "XMLInterface::insertStyleFileDefinition() - Could not open file...\n";
		return 0;
	}

	stream.seekp(43*sizeof(char),std::ios_base::beg);	// go to the correct position in the stream
	stream.write(styleDef.c_str(), 60*sizeof(char));	// write new line with xml-stylesheet definition
	stream.close();
	return 1;
}
