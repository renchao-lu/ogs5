/**
 * \file XMLInterface.h
 * 18/02/2010 KR Initial implementation
 */

#ifndef XMLINTERFACE_H
#define XMLINTERFACE_H

#include "Configure.h"

#include <map>
#include <QtXml/QXmlStreamReader>
#if OGS_QT_VERSION > 45
	#include <QtXmlPatterns/QXmlSchema>
#endif // OGS_QT_VERSION > 45
#include <QFile>

#include "GEOObjects.h"
#include "Point.h"
#include "Polyline.h"
#include "Surface.h"

/** 
 * The class is an interface between GeoObjects and XML files.
 */
class XMLInterface
{
public:
	/** 
	 * Constructor
	 * \param geoObjects An GEOObject that into which data will be read or from which data will be written.
	 * \param schemaFile An XML schema file (*.xsd) that defines the structure of a valid data file.
	 */
	XMLInterface(GEOLIB::GEOObjects* geoObects, std::string schemaFile);

	/// As QXMLStreamWriter seems currently unable to include style-file links into xml-files, this method will workaround this issue and include the stylefile link.
	int insertStyleFileDefinition(const QString &fileName);

	/// Check if the given xml-file is valid considering the schema-file used in the constructor
	int isValid(const QString &fileName) const;

	/// Reads an xml-file containing geometric object definitions into the GEOObjects used in the contructor
	int readGLIFile(const QString &fileName);

	/// Sets the schema file used to check if xml files are valid.
#if OGS_QT_VERSION > 45
	int setSchema(const std::string &schemaFile);
#endif // OGS_QT_VERSION > 45

	/**
	 * Writes data from GEOObjects to an xml-file
	 * \param file The file into which the data will be written.
	 * \param gliName The name of the GEOOBjects that will be written into the file.
	 */
	void writeGLIFile(QFile* file, const QString &gliName);

private:
	/// Reads GEOLIB::Point-objects from an xml-file
	void readPoints    ( QXmlStreamReader &xmlReader, std::vector<GEOLIB::Point*> *points );	

	/// Reads GEOLIB::Polyline-objects from an xml-file
	void readPolylines ( QXmlStreamReader &xmlReader, std::vector<GEOLIB::Polyline*> *polylines, std::vector<GEOLIB::Point*> *points );

	/// Reads GEOLIB::Surface-objects from an xml-file
	void readSurfaces  ( QXmlStreamReader &xmlReader, std::vector<GEOLIB::Surface*> *surfaces, std::vector<GEOLIB::Polyline*> *polylines );

	/// Returns the value of element 'name' as a string.
	QString readElement(QXmlStreamReader &xml, const QString &name) const;

	GEOLIB::GEOObjects* _geoObjects;
	
#if OGS_QT_VERSION > 45
	QXmlSchema _schema;
#endif // OGS_QT_VERSION > 45
	std::map<size_t, size_t> _idx_map;
};

#endif // XMLINTERFACE_H
