/**
 * \file VtkSurfacesSource.cpp
 * 3/2/2010 LB Initial implementation
 * 23/04/2010 KR Surface visualisation
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
#include <vtkSmartPointer.h>

vtkStandardNewMacro(VtkSurfacesSource);
vtkCxxRevisionMacro(VtkSurfacesSource, "$Revision$");

VtkSurfacesSource::VtkSurfacesSource()
: _surfaces(NULL)
{
	this->SetNumberOfInputPorts(0);
}

void VtkSurfacesSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_surfaces->size() == 0)
		return;

	os << indent << "== VtkSurfacesSource ==" << "\n";
}

int VtkSurfacesSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	const int nSurfaces = _surfaces->size();
	if (nSurfaces == 0)
		return 0;

	vtkSmartPointer<vtkInformation> outInfo = outputVector->GetInformationObject(0);
	vtkSmartPointer<vtkPolyData> output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> newPolygons = vtkSmartPointer<vtkCellArray>::New();
	//newPolygons->Allocate(nSurfaces);

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

/*
	const std::vector<GEOLIB::Point*> *surfacePoints = (*_surfaces)[0]->getPointVec();
	for (std::vector<GEOLIB::Point*>::const_iterator it = surfacePoints->begin(); it != surfacePoints->end(); ++it)
	{
		double* coords = const_cast<double*>((*it)->getData());
		newPoints->InsertNextPoint(coords);
	}
*/

	int numberOfPointsInserted = 0;
	for (std::vector<GEOLIB::Surface*>::const_iterator it = _surfaces->begin();
		it != _surfaces->end(); ++it)
	{
		const size_t nTriangles = (*it)->getNTriangles();
		vtkPolygon* aPolygon = vtkPolygon::New();
		aPolygon->GetPointIds()->SetNumberOfIds(nTriangles*3);

		for (size_t i = 0; i < nTriangles; i++)
		{
			const GEOLIB::Triangle* triangle = (**it)[i];
			for (size_t j=0; j<3; j++) 
			{
				double* coords = const_cast<double*>(triangle->getPoint(j)->getData());
				newPoints->InsertNextPoint(coords);
				aPolygon->GetPointIds()->SetId(i*3+j, numberOfPointsInserted);
				numberOfPointsInserted++;
			}
		}

		newPolygons->InsertNextCell(aPolygon);
		aPolygon->Delete();
	}

	output->SetPoints(newPoints);
	output->SetPolys(newPolygons);


	return 1;
}

int VtkSurfacesSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
