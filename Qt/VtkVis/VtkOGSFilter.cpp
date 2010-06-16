/**
 * \file VtkOGSFilter.cpp
 * 14/04/2010 KR Initial implementation
 *
 */


#include "VtkOGSFilter.h"

/* VTK Includes */
#include <vtkPolyDataAlgorithm.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkGlyph3D.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkThreshold.h>
#include <vtkTubeFilter.h>

/* Qt Includes */
#include <QImage>
#include <QPointF>
#include <QFileDialog>
#include <QFileInfo>

/* OGS Includes */
#include "OGSRaster.h"
#include "VtkColorByHeightFilter.h"
#include "VtkMeshSource.h"
#include "VtkTextureOnSurfaceFilter.h"

VtkOGSFilter::VtkOGSFilter()
{
	_availableFilters.push_back(VtkFilterItem("Apply Texture to Grid",	    VtkOGSFilter::TEXTOGRIDFILTER,		    VtkFilterItem::UNSTRUCTUREDGRID));
	_availableFilters.push_back(VtkFilterItem("Apply Texture to Surface",   VtkOGSFilter::TEXTOSURFACEFILTER,		VtkFilterItem::POLYDATA));
	_availableFilters.push_back(VtkFilterItem("Convert Lines to Cylinders", VtkOGSFilter::LINETOCYLINDERFILTER,     VtkFilterItem::POLYDATA));
	_availableFilters.push_back(VtkFilterItem("Convert Points to Glyphs",   VtkOGSFilter::POINTTOGLYPHFILTER,       VtkFilterItem::POLYDATA));
	_availableFilters.push_back(VtkFilterItem("Elevation-based Colour",     VtkOGSFilter::COLORBYHEIGHTFILTER_GRID, VtkFilterItem::UNSTRUCTUREDGRID));
	_availableFilters.push_back(VtkFilterItem("Elevation-based Colour",     VtkOGSFilter::COLORBYHEIGHTFILTER_POLY, VtkFilterItem::POLYDATA));
	_availableFilters.push_back(VtkFilterItem("Generate Surface",           VtkOGSFilter::SURFACEFILTER,            VtkFilterItem::UNSTRUCTUREDGRID));
	_availableFilters.push_back(VtkFilterItem("Show Material Groups",	    VtkOGSFilter::MATGROUPFILTER,		    VtkFilterItem::UNSTRUCTUREDGRID));
	//_availableFilters.push_back(VtkFilterItem("Specify Thresholds (currently not working correctly",         VtkOGSFilter::THRESHOLDINGFILTER,		VtkFilterItem::UNSTRUCTUREDGRID));
}

VtkOGSFilter::~VtkOGSFilter()
{
}

vtkAlgorithm* VtkOGSFilter::apply(vtkAlgorithm* input, OGSVisFilter filter)
{
	switch (filter)
	{
	case POINTTOGLYPHFILTER:
		return Point2GlyphFilter(static_cast<vtkPolyDataAlgorithm*>(input));
	case LINETOCYLINDERFILTER:
		return Line2CylinderFilter(static_cast<vtkPolyDataAlgorithm*>(input));
	case SURFACEFILTER:
		return SurfaceFilter(static_cast<vtkUnstructuredGridAlgorithm*>(input));
	case COLORBYHEIGHTFILTER_GRID:
		return ColorByHeight(static_cast<vtkUnstructuredGridAlgorithm*>(input));
	case COLORBYHEIGHTFILTER_POLY:
		return ColorByHeight(static_cast<vtkPolyDataAlgorithm*>(input));
	case MATGROUPFILTER:
		return MaterialGroupFilter(static_cast<vtkUnstructuredGridAlgorithm*>(input));
	case TEXTOGRIDFILTER:
		return TextureToGridFilter(static_cast<vtkUnstructuredGridAlgorithm*>(input));
	case TEXTOSURFACEFILTER:
		return TextureToSurfaceFilter(static_cast<vtkPolyDataAlgorithm*>(input));
	case THRESHOLDINGFILTER:
		return ThresholdFilter(static_cast<vtkUnstructuredGridAlgorithm*>(input),5,25);
	default:
		return input;
	}
}

vtkPolyDataAlgorithm* VtkOGSFilter::Point2GlyphFilter(vtkPolyDataAlgorithm* algorithm, double radius)
{
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
		sphere->SetRadius(radius);
		sphere->SetPhiResolution(10);
		sphere->SetThetaResolution(10);
		sphere->SetReleaseDataFlag(1);

	vtkGlyph3D* glyphs = vtkGlyph3D::New();
		glyphs->ScalingOn();
		glyphs->SetScaleModeToScaleByScalar();
	    glyphs->SetSource(sphere->GetOutput());
		glyphs->SetInput(algorithm->GetOutput());

	return glyphs;
}

vtkPolyDataAlgorithm* VtkOGSFilter::Line2CylinderFilter(vtkPolyDataAlgorithm* algorithm, double radius)
{
	vtkTubeFilter* tubes = vtkTubeFilter::New();
		tubes->SetInputConnection(0, algorithm->GetOutputPort(0));
		tubes->SetRadius(radius);
		tubes->SetNumberOfSides(10);
	return tubes;
}

vtkUnstructuredGridAlgorithm* VtkOGSFilter::MaterialGroupFilter(vtkUnstructuredGridAlgorithm* algorithm)
{
	VtkMeshSource* meshsource = static_cast<VtkMeshSource*>(algorithm);
		meshsource->setColorsFromMaterials();
		meshsource->SetScalarVisibility(true);
	return meshsource;
}

vtkPolyDataAlgorithm* VtkOGSFilter::SurfaceFilter(vtkUnstructuredGridAlgorithm* algorithm)
{
	vtkDataSetSurfaceFilter* surfaceFilter = vtkDataSetSurfaceFilter::New();
		surfaceFilter->SetInputConnection(0, algorithm->GetOutputPort(0));	
		surfaceFilter->Update();
	return surfaceFilter;
}

vtkPolyDataAlgorithm* VtkOGSFilter::ColorByHeight(vtkUnstructuredGridAlgorithm* algorithm)
{
	VtkMeshSource* meshsource = static_cast<VtkMeshSource*>(algorithm);
		meshsource->SetScalarVisibility(true);
	vtkPolyDataAlgorithm* surface = VtkOGSFilter::SurfaceFilter(meshsource);
	return VtkOGSFilter::ColorByHeight(surface);
}

vtkPolyDataAlgorithm* VtkOGSFilter::ColorByHeight(vtkPolyDataAlgorithm* algorithm)
{
	VtkColorByHeightFilter* heightFilter = VtkColorByHeightFilter::New();
		heightFilter->SetInputConnection(0, algorithm->GetOutputPort(0));
		heightFilter->GetColorLookupTable()->setMinRange(-35);
		heightFilter->GetColorLookupTable()->setMaxRange(800);
		//heightFilter->GetColorLookupTable()->setInterpolationType(ColorLookupTable::EXPONENTIAL);
	GEOLIB::Color* c = new GEOLIB::Color(0,255,0);
	GEOLIB::Color* d = new GEOLIB::Color(255,255,0);
		heightFilter->GetColorLookupTable()->setColor(c, 0.2);
		heightFilter->GetColorLookupTable()->setColor(d, 0.6);
		heightFilter->Update(); 

	return heightFilter;
}

vtkPolyDataAlgorithm* VtkOGSFilter::TextureToGridFilter(vtkUnstructuredGridAlgorithm* algorithm)
{
	return TextureToSurfaceFilter(VtkOGSFilter::SurfaceFilter(algorithm));
}

vtkPolyDataAlgorithm* VtkOGSFilter::TextureToSurfaceFilter(vtkPolyDataAlgorithm* algorithm)
{
	QWidget* parent = 0;
	QString fileName = QFileDialog::getOpenFileName(parent, "Select raster file to apply as texture", "","Raster files (*.asc *.bmp *.jpg *.png *.tif);;");
	QFileInfo fi(fileName);

	if ((fi.suffix().toLower() == "asc") || (fi.suffix().toLower() == "tif") || (fi.suffix().toLower() == "png") || 
		(fi.suffix().toLower() == "jpg") || (fi.suffix().toLower() == "bmp"))
	{
		QImage img;
		QPointF origin;
		double scalingFactor=0;

		OGSRaster::loadImage(fileName, img, origin, scalingFactor);
		std::pair<float, float> org(origin.x(), origin.y()); 
		VtkTextureOnSurfaceFilter* surface = VtkTextureOnSurfaceFilter::New();
			surface->SetRaster(img, org, scalingFactor);
			surface->SetInputConnection(0, algorithm->GetOutputPort(0));
			surface->Update();
		return surface;
		
	}
	return NULL;
}

vtkUnstructuredGridAlgorithm* VtkOGSFilter::ThresholdFilter(vtkUnstructuredGridAlgorithm* algorithm, size_t min, size_t max)
{
	vtkThreshold* threshold = vtkThreshold::New();
		//threshold->SetAttributeModeToUseCellData();
		//threshold->SetComponentModeToUseSelected();
		threshold->SetInputConnection(0, algorithm->GetOutputPort(0));
	//	threshold->ThresholdByLower(1);
		threshold->ThresholdBetween(min,max);
		threshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, static_cast<VtkMeshSource*>(algorithm)->GetMaterialArrayName());
		threshold->Update();
	
	return threshold;
}



/* VtkFilterItem */

VtkFilterItem::VtkFilterItem(std::string name, VtkOGSFilter::OGSVisFilter filter, VtkTargetObject target)
: _name(name), _filter(filter), _target(target) {}