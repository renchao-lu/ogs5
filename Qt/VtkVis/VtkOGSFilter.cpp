/**
 * \file VtkOGSFilter.cpp
 * 14/04/2010 KR Initial implementation
 *
 */


#include "VtkOGSFilter.h"

#include "VtkColorByHeightFilter.h"

#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkGlyph3D.h>
#include <vtkDataSetSurfaceFilter.h>


vtkPolyDataAlgorithm* VtkOGSFilter::Point2GlyphFilter(vtkPolyDataAlgorithm* algorithm)
{
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
	sphere->SetRadius(50);
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

vtkPolyDataAlgorithm* VtkOGSFilter::Line2CylinderFilter(vtkPolyDataAlgorithm* algorithm)
{
	vtkSmartPointer<vtkCylinderSource> cyl = vtkSmartPointer<vtkCylinderSource>::New();
	cyl->SetRadius(50);
	cyl->SetReleaseDataFlag(1);

	vtkGlyph3D* glyphs = vtkGlyph3D::New();
    glyphs->ScalingOn();
    glyphs->SetScaleModeToScaleByScalar();

    glyphs->SetSource(cyl->GetOutput());
	glyphs->SetInput(algorithm->GetOutput());

	return glyphs;
}
/*
	vtkCylinderSource* source = vtkCylinderSource::New();
	source->SetCenter(0,0,0);
	source->SetRadius(2.0);
	source->SetHeight(70.0);
	source->SetResolution(20.0); // 100 is really good, 10 is kinda insufficient
 	_mapper->SetInputConnection(0, source->GetOutputPort(0));
*/


vtkPolyDataAlgorithm* VtkOGSFilter::ColorByHeight(vtkUnstructuredGridAlgorithm* algorithm)
{
	vtkDataSetSurfaceFilter* surfaceFilter = vtkDataSetSurfaceFilter::New();
	surfaceFilter->SetInputConnection(0, algorithm->GetOutputPort(0));
	surfaceFilter->Update(); 

	VtkColorByHeightFilter* heightFilter = VtkColorByHeightFilter::New();
	//heightFilter->SetLimits(-7000,3000);
	heightFilter->SetInputConnection(0, surfaceFilter->GetOutputPort(0));
	heightFilter->Update(); 

	return heightFilter;
}