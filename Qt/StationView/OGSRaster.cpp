/**
 * \file OGSRaster.cpp
 * 11/01/2010 KR Initial implementation
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <QFileInfo>
#include "OGSRaster.h"
#include "StringTools.h"
#include "OGSError.h"

// libgeotiff includes
#include "geo_tiffp.h"
#include "xtiffio.h"


#include <QTime>

using namespace std;

bool OGSRaster::loadPixmap(const QString &fileName, QPixmap &raster, QPointF &origin, double &scalingFactor)
{
	QFileInfo fileInfo(fileName);
	origin.setX(0);
	origin.setY(0);
	scalingFactor = 1;

    if (fileInfo.suffix().toLower() == "asc")
	{
		if (!loadPixmapFromASC(fileName, raster, origin, scalingFactor)) return false;
		else raster = raster.transformed(QTransform(1, 0, 0, -1, 0, 0), Qt::FastTransformation);
	}
	if (fileInfo.suffix().toLower() == "tif")
	{
		if (!loadPixmapFromTIFF(fileName, raster, origin, scalingFactor)) return false;
	}
	else
	{
		if (!loadPixmapFromImage(fileName, raster)) return false;
		else raster = raster.transformed(QTransform(1, 0, 0, -1, 0, 0), Qt::FastTransformation);
	}
	return true;
}

bool OGSRaster::loadPixmapFromASC(const QString &fileName, QPixmap &raster, QPointF &origin, double &cellsize)
{
	ifstream in( fileName.toStdString().c_str() );

	if (!in.is_open())
    {
		cout << "OGSRaster::loadPixmapFromASC() - Could not open file...\n";
		return false;
	}

	ascHeader header;

	if (readASCHeader(header, in))
	{
		int index=0, gVal;
		double value, minVal=pow(2.0,16), maxVal=0;
		double *pixVal (new double[header.ncols * header.nrows]);
		QImage img(header.ncols, header.nrows, QImage::Format_ARGB32);

		QTime myTimer;
    	 myTimer.start();

		 string s;
		// read the file into an int-array
		for (int j=0; j<header.nrows; j++)
		{
			index = j*header.ncols;
			for (int i=0; i<header.ncols; i++)
			{
				in >> s;
				pixVal[index+i] = strtod(replaceString(",", ".", s).c_str(),0);
				if (pixVal[index+i] != header.noData)
				{	// find intensity bounds but ignore noData values
					minVal = (pixVal[index+i]<minVal) ? pixVal[index+i] : minVal;
					maxVal = (pixVal[index+i]>maxVal) ? pixVal[index+i] : maxVal;
				}
			}
		}
		in.close();

		std::cout << "Reading raster file: " << myTimer.elapsed() << " ms" << std::endl;


		// calculate scaling factor for contrast stretching
		double scalingFactor = 255.0/(maxVal-minVal);

		// write re-calculated image intensities to QImage
		// the formula used for contrast adjustment is p_new = (value - minval) * (g_max-g_min)/(maxval-minval)
		for (int j=0; j<header.nrows; j++)
		{
			index = j*header.ncols;
			for (int i=0; i<header.ncols; i++)
			{	// scale intensities and set nodata values to white (i.e. the background colour)
				value = (pixVal[index+i]==header.noData) ? maxVal : pixVal[index+i];
				gVal = static_cast<int> (floor((value-minVal)*scalingFactor));
				img.setPixel(i,j, qRgb(gVal, gVal, gVal));
			}
		}

		delete []pixVal;
		origin.setX(header.x);
		origin.setY(header.y);
		cellsize = header.cellsize;
		raster = QPixmap::fromImage(img);
		return true;
	}
	OGSError::box("Error reading file header.");
	return false;
}


bool OGSRaster::readASCHeader(ascHeader &header, ifstream &in)
{
	string line, tag, value;

	in >> tag;
	if (tag.compare("ncols")==0) { in >> value; header.ncols = atoi(value.c_str()); }
	else return false;
	in >> tag;
	if (tag.compare("nrows")==0) { in >> value; header.nrows = atoi(value.c_str()); }
	else return false;
	in >> tag;
	if (tag.compare("xllcorner")==0) { in >> value; header.x = strtod(replaceString(",", ".", value).c_str(),0); }
	else return false;
	in >> tag;
	if (tag.compare("yllcorner")==0) { in >> value; header.y = strtod(replaceString(",", ".", value).c_str(),0); }
	else return false;
	in >> tag;
	if (tag.compare("cellsize")==0) { in >> value; header.cellsize = strtod(replaceString(",", ".", value).c_str(),0); }
	else return false;
	in >> tag;
	if (tag.compare("NODATA_value")==0) { in >> value; header.noData = atoi(value.c_str()); }
	else return false;

	return true;
}

bool OGSRaster::loadPixmapFromTIFF(const QString &fileName, QPixmap &raster, QPointF &origin, double &cellsize)
{
	TIFF* tiff = XTIFFOpen(fileName.toStdString().c_str(), "r");

	if (tiff)
    {
		GTIF* geoTiff = GTIFNew(tiff);

		if (geoTiff)
		{

			int imgWidth=0, imgHeight=0, nImages=0, pntCount = 0;
			double* pnts = 0;

			// get actual number of images in the tiff file
			do {
				nImages++;
			} while (TIFFReadDirectory(tiff));

			// get image size
			TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH,  &imgWidth);
			TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &imgHeight);

			// get upper left point / origin
			if (TIFFGetField(tiff, GTIFF_TIEPOINTS, &pntCount, &pnts))
			{
				origin.setX(pnts[3]);
				origin.setY(pnts[4]);
			}

			// get cellsize
			// Note: GeoTiff allows anisotropic pixels. This is not supported here and equilateral pixels are assumed.
			if (TIFFGetField(tiff, GTIFF_PIXELSCALE, &pntCount, &pnts))
			{
				if (pnts[0] != pnts[1]) std::cout << "OGSRaster::loadPixmapFromTIFF() - Warning: Original raster data has anisotrop pixel size!" << endl;
				cellsize = pnts[0];
			}

			uint32 *pixVal = (uint32*) _TIFFmalloc(imgWidth*imgHeight * sizeof (uint32));

			if ((imgWidth > 0) && (imgHeight > 0))
			{
				if (!TIFFReadRGBAImage(tiff, imgWidth, imgHeight, pixVal, 0))
				{
					std::cout << "OGSRaster::loadPixmapFromTIFF() - Error reading GeoTIFF file." << endl;
					_TIFFfree(pixVal);
					GTIFFree(geoTiff);
					XTIFFClose(tiff);
					return false;
				}
			}

			int* pxl (new int[4]);
			int index=0;
			QImage img(imgWidth, imgHeight, QImage::Format_ARGB32);

			for (int j=0; j<imgHeight; j++)
			{
				index = j*imgWidth;
				for (int i=0; i<imgWidth; i++)
				{	// scale intensities and set nodata values to white (i.e. the background colour)
					uint32toRGBA(pixVal[index+i], pxl);
					img.setPixel(i,j, qRgba(pxl[0], pxl[1], pxl[2], pxl[3]));
				}
			}

			delete []pxl;

			raster = QPixmap::fromImage(img);

			_TIFFfree(pixVal);
			GTIFFree(geoTiff);
			XTIFFClose(tiff);
			return true;
		}

		XTIFFClose(tiff);
		std::cout << "OGSRaster::loadPixmapFromTIFF() - File not recognised as GeoTIFF-Image." << endl;
		return false;
	}

	std::cout << "OGSRaster::loadPixmapFromTIFF() - File not recognised as TIFF-Image." << endl;
	return false;
}


bool OGSRaster::loadPixmapFromImage(const QString &fileName, QPixmap &raster)
{
	return raster.load(fileName);
}

void OGSRaster::convertToGreyscale(QPixmap &raster, const int &min, const int &max)
{
	int value = 0;
	double scalingFactor = 255.0/(max-min);

	QImage img = raster.toImage();
	for (int i=0; i<img.width(); i++)
	{
		for (int j=0; j<img.height(); j++)
		{
			QRgb pix = img.pixel(i,j);
			value = static_cast<int>(floor(((0.3*qRed(pix)+0.6*qGreen(pix)+0.1*qBlue(pix))-min)*scalingFactor));
			img.setPixel(i, j, qRgb(value, value, value));
		}
	}
	raster = QPixmap::fromImage(img);
}


int* OGSRaster::getGreyscaleData(QPixmap &raster, const int &min, const int &max)
{
	int index = 0;
	double scalingFactor = 255.0/(max-min);
	int *pixVal (new int[raster.height() * raster.width()]);
	
	QImage img = raster.toImage();
	for (int j=0; j<img.height(); j++)
	{
		index = j*raster.width();
		for (int i=0; i<img.width(); i++)
		{
			QRgb pix = img.pixel(i,j);
			pixVal[index+i] = static_cast<int>(floor(((0.3*qRed(pix)+0.6*qGreen(pix)+0.1*qBlue(pix))-min)*scalingFactor));
		}
	}
	return pixVal;
}


int OGSRaster::getMaxValue(const QPixmap &raster)
{
	int value, maxVal = 0;
	QImage img = raster.toImage();
	for (int i=0; i<img.width(); i++)
	{
		for (int j=0; j<img.height(); j++)
		{
			value = qGreen(img.pixel(i,j));
			maxVal = (value>maxVal) ?  value : maxVal;
		}
	}
	return maxVal;
}

int OGSRaster::getMinValue(const QPixmap &raster)
{
	int value, minVal = static_cast <int> (pow(2.0,16));
	QImage img = raster.toImage();
	for (int i=0; i<img.width(); i++)
	{
		for (int j=0; j<img.height(); j++)
		{
			value = qGreen(img.pixel(i,j));
			minVal = (value<minVal) ? value : minVal;
		}
	}
	return minVal;
}

void OGSRaster::uint32toRGBA(const unsigned int s, int* p)
{
	p[3]   = s / (256*256*256);
	int r  = s % (256*256*256);
	p[2]   = r / (256*256);
	r     %= (256 * 256);
	p[1]   = r / 256;
	p[0]   = r % 256;
}
