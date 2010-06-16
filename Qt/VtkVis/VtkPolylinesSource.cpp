/**
 * \file VtkPolylinesSource.cpp
 * 2/2/2010 LB Initial implementation
 * 
 * Implementation of VtkPolylinesSource
 */

// ** INCLUDES **
#include "VtkPolylinesSource.h"

#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(VtkPolylinesSource);
vtkCxxRevisionMacro(VtkPolylinesSource, "$Revision$");

VtkPolylinesSource::VtkPolylinesSource()
: _polylines(NULL)
{
	this->SetNumberOfInputPorts(0);

	GEOLIB::Color* c = GEOLIB::getRandomColor();
	GetProperties()->SetColor((*c)[0]/255.0,(*c)[1]/255.0,(*c)[2]/255.0);
}

void VtkPolylinesSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_polylines->size() == 0)
		return;

	for (std::vector<GEOLIB::Polyline*>::const_iterator it = _polylines->begin();
		it != _polylines->end(); ++it)
	{
		os << indent << "== Polyline ==" << "\n";
		int numPoints = (*it)->getSize();
		for (int i = 0; i < numPoints; i++)
		{
			const GEOLIB::Point* point = (**it)[i];
			const double* coords = point->getData();
			os << indent << "Point " << i <<" (" << coords[0] << ", " << coords[1] << ", " << coords[2] << ")\n";
		}
	}
	
}

int VtkPolylinesSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	if (!_polylines)
		return 0;
	if (_polylines->size() == 0)
	{
		std::cout << "ERROR in VtkPolylineSource::RequestData : Size of polyline vector is 0" << std::endl;
		return 0;
	}

	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkPoints* newPoints = vtkPoints::New();
	vtkCellArray* newLines = vtkCellArray::New();

	//newPoints->Allocate(numPoints);
	//newLines->Allocate(newLines->EstimateSize(numLines, 2));

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;
	int lastMaxIndex = 0;
	for (std::vector<GEOLIB::Polyline*>::const_iterator it = _polylines->begin();
		it != _polylines->end(); ++it)
	{
		const int numPoints = (*it)->getSize();
		const int numLines = numPoints - 1;

		// Generate points
		for (int i = 0; i < numPoints; i++)
		{
			const GEOLIB::Point* point = (**it)[i];
			const double* coords = point->getData();
			newPoints->InsertNextPoint(coords);
		}

		// Generate lines
		newLines->InsertNextCell(numPoints);
		for (int i = 0; i < numPoints; i++)
			newLines->InsertCellPoint(i + lastMaxIndex);

		lastMaxIndex += numPoints;
	}

	output->SetPoints(newPoints);
	newPoints->Delete();

	output->SetLines(newLines);
	newLines->Delete();

	return 1;
}

int VtkPolylinesSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
