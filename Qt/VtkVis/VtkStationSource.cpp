/**
 * \file VtkStationSource.cpp
 * 24/02/2010 KR Initial implementation
 *
 */

#include "Station.h"
#include "Color.h"

// ** VTK INCLUDES **
#include "VtkStationSource.h"

#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkObjectFactory.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>

vtkStandardNewMacro(VtkStationSource);
vtkCxxRevisionMacro(VtkStationSource, "$Revision$");

VtkStationSource::VtkStationSource()
: _stations(NULL)
{
	this->SetNumberOfInputPorts(0);
}

void VtkStationSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_stations->size() == 0)
		return;

	os << indent << "== VtkStationSource ==" << "\n";

	int i = 0;
	for (std::vector<GEOLIB::Point*>::const_iterator it = _stations->begin();
		it != _stations->end(); ++it)
	{
		const double* coords = (*it)->getData();
		os << indent << "Station " << i <<" (" << coords[0] << ", " << coords[1] << ", " << coords[2] << ")\n";
		i++;
	}
}


/// Create 3d Station objects
int VtkStationSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	if (!_stations)
		return 0;
	int nStations = _stations->size();
	if (nStations == 0)
		return 0;

	bool isBorehole = (static_cast<GEOLIB::Station*>((*_stations)[0])->type() == GEOLIB::Station::BOREHOLE) ? true : false;


	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkPoints* newStations = vtkPoints::New();
	vtkCellArray* newVerts = vtkCellArray::New();
	//newStations->Allocate(nStations);
	//newVerts->Allocate(nStations);

	vtkCellArray* newLines;
	if (isBorehole) newLines = vtkCellArray::New();

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	// create colour (this assumes that all points in a list have the same colour.
	// if this is not the case the colour for each point has to be set for each point
	// individually in the loop below
	GEOLIB::Color* c = static_cast<GEOLIB::Station*>((*_stations)[0])->getColor();
	unsigned char color[3] = {(*c)[0], (*c)[1], (*c)[2]};

	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("StationColors");
	colors->InsertNextTupleValue(color);

	vtkSmartPointer<vtkUnsignedCharArray> stratColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	stratColors->SetNumberOfComponents(3);
	colors->SetName("StratColors");

	int lastMaxIndex = 0;

	bool notTooLarge = true;

	// Generate graphic objects
	for (std::vector<GEOLIB::Point*>::const_iterator it = _stations->begin();
		it != _stations->end(); ++it)
	{
		const double* coords = (*it)->getData();
		vtkIdType sid[1];
		sid[0] = newStations->InsertNextPoint(coords);
		newVerts->InsertNextCell(1, sid);
		colors->InsertNextTupleValue(color);

//		if (lastMaxIndex>4000)
//		{
//			notTooLarge = false;
//			lastMaxIndex++;
//		} else notTooLarge=true;

		int scalingFactor=1;
		if (isBorehole && notTooLarge)
		{
			GEOLIB::StationBorehole* bore = static_cast<GEOLIB::StationBorehole*>(*it);
			std::vector<GEOLIB::Point*> profile = bore->getProfile();
			const size_t nLayers = profile.size();

			// Generate points (the 0-th point is the station itself and has already been added)
			for (size_t i=1; i<nLayers; i++)
			{
				const double* coords = profile[i]->getData();
				double c[3];
				c[0] = coords[0];c[1] = coords[1];c[2] = coords[2]*scalingFactor;
				newStations->InsertNextPoint(c);
			}

			// Generate lines
			newLines->InsertNextCell(nLayers);
			for (size_t i=0; i<nLayers; i++)
			{
				newLines->InsertCellPoint(lastMaxIndex+i);
				GEOLIB::Color* c = GEOLIB::getRandomColor();
				unsigned char randomColor[3] = {(*c)[0], (*c)[1], (*c)[2]};
				stratColors->InsertNextTupleValue(randomColor);
			}

			lastMaxIndex += (nLayers);
		}
	}
/*
	for (size_t i=0; i<_stations->size(); i++)
		colors->InsertNextTupleValue(color);
	
	for (int i=0; i<newStations->GetData()->GetDataSize(); i++)
	{
		colors->InsertNextTupleValue();
	}
*/
	output->SetPoints(newStations);
	newStations->Delete();

	output->SetVerts(newVerts);
	output->GetCellData()->SetScalars(colors);
	newVerts->Delete();

	if (isBorehole)
	{
		output->SetLines(newLines);
		newLines->Delete();
	}

	int nColors = stratColors->GetDataSize(); 
	for (int i=0; i<nColors; i++)
	{
		unsigned char c[3];
		stratColors->GetTupleValue(i, c);
		colors->InsertNextTupleValue(c);
	}

	output->GetCellData()->AddArray(stratColors);

	return 1;
}

int VtkStationSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
