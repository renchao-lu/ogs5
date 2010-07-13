/**
 * \file XMLInterface.cpp
 * 18/02/2010 KR Initial implementation
 */

#include "XMLInterface.h"

#include <iostream>
#include <QFileInfo>

#include <QFile>
#include <QTextCodec>
#include <QCryptographicHash>
#include <QtXml/QDomDocument>
#if OGS_QT_VERSION > 45
	#include <QtXmlPatterns/QXmlSchemaValidator>
#endif // QT_VERSION > 45

#include <QTime>

XMLInterface::XMLInterface(GEOLIB::GEOObjects* geoObjects, std::string schemaFile) : _geoObjects(geoObjects)
{
#if OGS_QT_VERSION > 45
	this->setSchema(schemaFile);
#else
	Q_UNUSED (schemaFile)
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
			std::cout << "XMLInterface::isValid() - XML File is invalid (in reference to the given schema)." << std::endl;
			return 0;
        }
    } else {
        std::cout << "XMLInterface::isValid() - Schema is invalid." << std::endl;
		return -1;
    }
#else
    Q_UNUSED (fileName);
	std::cout << "XMLInterface: XML schema validation skipped. Qt 4.6 is required for validation." << std::endl;
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
	QString gliName = "";

	QFile* file = new QFile(fileName);
	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		std::cout << "XMLInterface::readGLIFile() - Can't open xml-file." << std::endl;
		delete file;
		return 0;
	}
	if (!checkHash(fileName)) { delete file; return 0; }

	std::vector<GEOLIB::Point*>    *points    = new std::vector<GEOLIB::Point*>;
	std::vector<GEOLIB::Polyline*> *polylines = new std::vector<GEOLIB::Polyline*>;
	std::vector<GEOLIB::Surface*>  *surfaces  = new std::vector<GEOLIB::Surface*>;

	QDomDocument doc("OGS-GLI-DOM");
	doc.setContent(file); 
	QDomElement docElement = doc.documentElement(); //OpenGeoSysGLI
	QDomNodeList geoTypes = docElement.childNodes();

	for(int i=0; i<geoTypes.count(); i++)
    {
		if (geoTypes.at(i).nodeName().compare("name") == 0)				gliName = geoTypes.at(i).toElement().text();
		else if (geoTypes.at(i).nodeName().compare("points") == 0)		readPoints(geoTypes.at(i), points);
		else if (geoTypes.at(i).nodeName().compare("polylines") == 0)	readPolylines(geoTypes.at(i), polylines, points);
		else if (geoTypes.at(i).nodeName().compare("surfaces") == 0)	readSurfaces(geoTypes.at(i), surfaces, points);
		else std::cout << "Unknown XML-Node found in file." << std::endl;
	}
	delete file;

	std::string gliFinalName = gliName.toStdString();
	_geoObjects->addPointVec(points, gliFinalName);
	_geoObjects->addPolylineVec(polylines, gliFinalName);
	_geoObjects->addSurfaceVec(surfaces, gliFinalName);
	return 1;
}

void XMLInterface::readPoints( const QDomNode &pointsRoot, std::vector<GEOLIB::Point*> *points )
{
	char* pEnd;
	QDomElement point = pointsRoot.firstChildElement();
	while (!point.isNull()) 
	{
		if (point.hasAttribute("id") && point.hasAttribute("x") && point.hasAttribute("y"))
		{
			_idx_map.insert (std::pair<size_t,size_t>(strtol((point.attribute("id")).toStdString().c_str(), &pEnd, 10), points->size()));
			double zVal = (point.hasAttribute("z")) ? strtod((point.attribute("z")).toStdString().c_str(), 0) : 0.0;
			GEOLIB::Point* p = new GEOLIB::Point(strtod((point.attribute("x")).toStdString().c_str(), 0),
												 strtod((point.attribute("y")).toStdString().c_str(), 0),
												 zVal);
			points->push_back(p);
		} 
		else std::cout << "XMLInterface::readPoints() - Attribute missing in <point> tag ..." << std::endl;
		point = point.nextSiblingElement();
	}
}

void XMLInterface::readPolylines( const QDomNode &polylinesRoot, std::vector<GEOLIB::Polyline*> *polylines, std::vector<GEOLIB::Point*> *points )
{
	size_t idx=0;
	QDomElement polyline = polylinesRoot.firstChildElement();
	while (!polyline.isNull())
	{
		if (polyline.hasAttribute("id"))
		{
			idx = polylines->size();
			polylines->push_back(new GEOLIB::Polyline(*points));

			QDomElement point = polyline.firstChildElement();
			while (!point.isNull())
			{
				(*polylines)[idx]->addPoint(_idx_map[atoi(point.text().toStdString().c_str())]);
				point = point.nextSiblingElement();
			}
		} 
		else std::cout << "XMLInterface::readPolylines() - Attribute missing in <polyline> tag ..." << std::endl;
		polyline = polyline.nextSiblingElement();
	}
}

void XMLInterface::readSurfaces( const QDomNode &surfacesRoot, std::vector<GEOLIB::Surface*> *surfaces, std::vector<GEOLIB::Point*> *points )
{
	QDomElement surface = surfacesRoot.firstChildElement();
	while (!surface.isNull())
	{
		if (surface.hasAttribute("id"))
		{
			surfaces->push_back(new GEOLIB::Surface(*points));

			QDomElement element = surface.firstChildElement();
			while (!element.isNull())
			{
				if (element.hasAttribute("p1") && element.hasAttribute("p2") && element.hasAttribute("p3"))
				{
					size_t p1 = _idx_map[atoi((element.attribute("p1")).toStdString().c_str())];
					size_t p2 = _idx_map[atoi((element.attribute("p2")).toStdString().c_str())];
					size_t p3 = _idx_map[atoi((element.attribute("p3")).toStdString().c_str())];
					surfaces->back()->addTriangle(p1,p2,p3);
				}
				else std::cout << "XMLInterface::readSurfaces() - Attribute missing in <element> tag ..." << std::endl;
				element = element.nextSiblingElement();
			}
		}
		else std::cout << "XMLInterface::readSurfaces() - Attribute missing in <surface> tag ..." << std::endl;
		surface = surface.nextSiblingElement();
	}
}

void XMLInterface::writeGLIFile(QFile &file, const QString &gliName)
{
	size_t nPoints=0, nPolylines=0, nSurfaces=0;

	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.setCodec(QTextCodec::codecForName("ISO-8859-1"));

	xml.writeCharacters("<!-- hash code -->\n");

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
	const std::vector<GEOLIB::Point*> *points (_geoObjects->getPointVec(gliName.toStdString()));
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
	const std::vector<GEOLIB::Polyline*> *polylines (_geoObjects->getPolylineVec(gliName.toStdString()));
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
	const std::vector<GEOLIB::Surface*> *surfaces (_geoObjects->getSurfaceVec(gliName.toStdString()));
	nSurfaces = surfaces->size();
	for (size_t i=0; i<nSurfaces; i++)
	{
		xml.writeStartElement("surface");
		xml.writeAttribute("id", QString::number(i));

		// writing the elements compromising the surface
		size_t nElements = (*surfaces)[i]->getNTriangles();
		for (size_t j=0; j<nElements; j++)
		{
			xml.writeStartElement("element"); //triangle-element
			xml.writeAttribute("p1", QString::number((*(*(*surfaces)[i])[j])[0]));
			xml.writeAttribute("p2", QString::number((*(*(*surfaces)[i])[j])[1]));
			xml.writeAttribute("p3", QString::number((*(*(*surfaces)[i])[j])[2]));
			xml.writeEndElement(); //triangle-element
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

bool XMLInterface::checkHash(const QString &fileName) const
{
	QFileInfo fi(fileName);
	QString md5FileName(fileName.left(fileName.length()-3) + "md5");
	std::string md5HashStr;

	std::ifstream md5( md5FileName.toStdString().c_str() );
	if (md5.is_open())
	{
		getline(md5, md5HashStr);
		QByteArray md5Hash(md5HashStr.c_str());
		if (fileIsValid(fileName, md5Hash)) return true;
	}

	if (!this->isValid(fileName)) return false;

	std::cout << "File is valid, writing hashfile..." << std::endl;
	QByteArray hash = calcHash(fileName);
	std::ofstream out( md5FileName.toStdString().c_str(), std::ios::out );
	out << hash.data();
	out.close();
	return true;
}

QByteArray XMLInterface::calcHash(const QString &fileName) const
{
	int length;
	char * buffer;
	std::ifstream is;
	is.open (fileName.toStdString().c_str(), std::ios::binary );
	is.seekg (0, std::ios::end);
	length = is.tellg();
	is.seekg (0, std::ios::beg);
	buffer = new char [length];
	is.read (buffer,length);
	is.close();
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

bool XMLInterface::fileIsValid(const QString &fileName, const QByteArray &hash) const
{
	int hashLength = hash.length();
	QByteArray fileHash = calcHash(fileName);
	if (fileHash.length() != hashLength) return false;
	for (int i=0; i<hashLength; i++)
	{
		if (fileHash[i] != hash[i]) return false;
	}
	return true;
}
