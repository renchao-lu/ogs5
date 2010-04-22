/**
 * \file VtkColorByHeightFilter.cpp
 * 21/04/2010 KR Initial implementation
 *
 */

// ** VTK INCLUDES **
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkObjectFactory.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include "VtkColorByHeightFilter.h"

vtkStandardNewMacro(VtkColorByHeightFilter);
vtkCxxRevisionMacro(VtkColorByHeightFilter, "$Revision$");


VtkColorByHeightFilter::VtkColorByHeightFilter() : _min(DEFAULTMINVALUE), _max(DEFAULTMAXVALUE)
{
}

void VtkColorByHeightFilter::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "== VtkColorByHeightFilter ==" << endl;
	os << indent << "min value = " << _min << endl;
	os << indent << "max value = " << _max << endl;
}

int VtkColorByHeightFilter::RequestData( vtkInformation* request, 
							             vtkInformationVector** inputVector, 
								         vtkInformationVector* outputVector )
{
	vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkPolyData *input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	// Generate a color lookup table
	vtkSmartPointer<vtkLookupTable> colorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
	colorLookupTable->SetTableRange(getMinHeight(input), getMaxHeight(input));
	colorLookupTable->Build();

	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);

	// Generate the colors for each point based on the color map
	size_t nPoints = input->GetNumberOfPoints();
	for (size_t i=0; i<nPoints; i++)
	{
		double p[3];
		input->GetPoint(i,p);

		double lutColor[3];
		colorLookupTable->GetColor(p[2], lutColor);
		unsigned char color[3];
		for (unsigned int j=0; j<3; j++)
			color[j] = 255 * lutColor[j]/1.0;

		colors->InsertNextTupleValue(color);
	}

	output->CopyStructure(input);
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());

	colors->SetName("Colors");
	output->GetPointData()->AddArray(colors);
	output->GetPointData()->SetActiveScalars("Colors");

	return 1;
}

double VtkColorByHeightFilter::getMinHeight(vtkPolyData* data)
{
	size_t nPoints = data->GetNumberOfPoints();
	if (_min == DEFAULTMINVALUE && nPoints > 0)
	{
		double p[3];
		data->GetPoint(0,p);
		_min = p[2];

		for (size_t i=1; i<nPoints; i++)
		{
			data->GetPoint(i,p);
			if (p[2]<_min) _min = p[2];
		}
	}
	return _min;
}

double VtkColorByHeightFilter::getMaxHeight(vtkPolyData* data)
{
	size_t nPoints = data->GetNumberOfPoints();
	if (_max == DEFAULTMAXVALUE && nPoints > 0)
	{
		double p[3];
		data->GetPoint(0,p);
		_max = p[2];

		for (size_t i=1; i<nPoints; i++)
		{
			data->GetPoint(i,p);
			if (p[2]>_max) _max = p[2];
		}
	}
	return _max;
}

void VtkColorByHeightFilter::SetLimits(double min, double max)
{
	if (min < max)
	{
		_min = min;
		_max = max;
	}
	else 
		vtkstd::cout << "VtkColorByHeightFilter::SetLimits(min, max) - Limits not changed because min value > max value." << vtkstd::endl;
}