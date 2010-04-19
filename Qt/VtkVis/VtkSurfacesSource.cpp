/**
 * \file VtkSurfacesSource.cpp
 * 3/2/2010 LB Initial implementation
 *
 * Implementation of VtkSurfacesSource
 */

// ** INCLUDES **
#include "VtkSurfacesSource.h"

#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolygon.h>

vtkStandardNewMacro(VtkSurfacesSource);
vtkCxxRevisionMacro(VtkSurfacesSource, "$Revision$");

VtkSurfacesSource::VtkSurfacesSource()
: _surfaces(NULL)
{
	this->SetNumberOfOutputPorts(0);
}

void VtkSurfacesSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_surfaces.size() == 0)
		return;

	os << indent << "== VtkSurfacesSource ==" << "\n";
}

int VtkSurfacesSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	const int numSurfaces = _surfaces.size();
	if (numSurfaces == 0)
		return 0;

	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkPoints* newPoints = vtkPoints::New();
	vtkCellArray* newPolygons = vtkCellArray::New();
	newPolygons->Allocate(numSurfaces);

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

//	for (std::vector<GEOLIB::Surface*>::const_iterator it = _surfaces.begin();
//		it != _surfaces.end(); ++it)
//	{
//		const size_t n_triangles = (*it)->getNTriangles();
//		vtkPolygon* newPolygon = vtkPolygon::New();
//
//		int numberOfPointsInserted = 0;
//		for (size_t i = 0; i < n_triangles; i++)
//		{
//			GEOLIB::Triangle* triangle = (**it)[i];
//			for (int j = 0; j < 3; j++) {
//				const GEOLIB::Point* point = (*polyline)[j];
//				const double* coords = point->getData();
//				newPoints->InsertNextPoint(coords);
//
//				newPolygon->GetPointIds()->InsertNextId(numberOfPointsInserted);
//
//				numberOfPointsInserted++;
//			}
//		}
//
//		newPolygons->InsertNextCell(newPolygon);
//		newPolygon->Delete();
//	}

	output->SetPoints(newPoints);
	newPoints->Delete();

	output->SetPolys(newPolygons);
	newPolygons->Delete();

	return 1;
}

int VtkSurfacesSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
