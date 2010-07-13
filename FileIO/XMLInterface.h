/**
 * \file XMLInterface.h
 * 18/02/2010 KR Initial implementation
 */

#ifndef XMLINTERFACE_H
#define XMLINTERFACE_H

#include "Configure.h"
#include "GEOObjects.h"

#include <QXmlStreamReader>
#if OGS_QT_VERSION > 45
	#include <QtXmlPatterns/QXmlSchema>
#endif // OGS_QT_VERSION > 45



class QFile;
class QDomNode;

/** 
 * \brief Reads and writes GeoObjects to and from XML files.
 */
class XMLInterface
{
public:
	/** 
	 * Constructor
	 * \param geoObjects An GEOObject that into which data will be read or from which data will be written.
	 * \param schemaFile An XML schema file (*.xsd) that defines the structure of a valid data file.
	 */
	XMLInterface(GEOLIB::GEOObjects* geoObjects, std::string schemaFile);

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
	void writeGLIFile(QFile &file, const QString &gliName);

private:
	/// Reads GEOLIB::Point-objects from an xml-file
	void readPoints    ( const QDomNode &pointsRoot, std::vector<GEOLIB::Point*> *points );	

	/// Reads GEOLIB::Polyline-objects from an xml-file
	void readPolylines ( const QDomNode &polylinesRoot, std::vector<GEOLIB::Polyline*> *polylines, std::vector<GEOLIB::Point*> *points );

	/// Reads GEOLIB::Surface-objects from an xml-file
	void readSurfaces  ( const QDomNode &surfacesRoot, std::vector<GEOLIB::Surface*> *surfaces, std::vector<GEOLIB::Point*> *points );

	/// Checks if a hash for the given data file exists to skip the time-consuming validation part.
	/// If a hash file exists _and_ the hash of the data file is the same as the content of the hash file the validation is skipped
	/// If no hash file exists, the xml-file is validated and a hash file is written if the xml-file was valid.
	bool checkHash(const QString &fileName) const;

	/// Calculates an MD5 hash of the given file.
	QByteArray calcHash(const QString &fileName) const;
	
	/// Checks if the given file is conform to the given hash.
	bool fileIsValid(const QString &fileName, const QByteArray &hash) const;

	GEOLIB::GEOObjects* _geoObjects;
	
#if OGS_QT_VERSION > 45
	QXmlSchema _schema;
#endif // OGS_QT_VERSION > 45
	std::map<size_t, size_t> _idx_map;
};

#endif // XMLINTERFACE_H
