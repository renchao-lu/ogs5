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


VtkColorByHeightFilter::VtkColorByHeightFilter()
{
	_colorLookupTable = new ColorLookupTable();
}

VtkColorByHeightFilter::~VtkColorByHeightFilter()
{
	delete _colorLookupTable;
}

void VtkColorByHeightFilter::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	os << indent << "== VtkColorByHeightFilter ==" << endl;
	os << indent << "Range: " << _colorLookupTable->getMinRange() << "-" << _colorLookupTable->getMaxRange() << endl;
	os << indent << "Interpolation Type:" << _colorLookupTable->getInterpolationType() << endl;
}

int VtkColorByHeightFilter::RequestData( vtkInformation* request, 
							             vtkInformationVector** inputVector, 
								         vtkInformationVector* outputVector )
{
	(void)request;

	vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkPolyData *input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
	
	// Generate a color lookup table
/*	// vtk filter for lut-generation (supports only standard hsv colour sequence
	vtkSmartPointer<vtkLookupTable> colorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
		colorLookupTable->SetTableRange(getMinHeight(input), getMaxHeight(input));
		colorLookupTable->Build();
*/
	// our own colour table generation class 
	_colorLookupTable->setMinRange(getMinHeight(input));
	_colorLookupTable->setMaxRange(getMaxHeight(input));
	_colorLookupTable->build();

	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
		colors->SetNumberOfComponents(3);
		colors->SetName("Colors");

	// Generate the colors for each point based on the color map
	size_t nPoints = input->GetNumberOfPoints();
	for (size_t i=0; i<nPoints; i++)
	{
		double p[3];
		input->GetPoint(i,p);

		unsigned char lutColor[3];
		_colorLookupTable->getColor((size_t)p[2], lutColor);
		colors->InsertNextTupleValue(lutColor);
	}

	vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
		output->CopyStructure(input);
		output->GetPointData()->PassData(input->GetPointData());
		output->GetCellData()->PassData(input->GetCellData());
		output->GetPointData()->AddArray(colors);
		output->GetPointData()->SetActiveScalars("Colors");

	return 1;
}

double VtkColorByHeightFilter::getMinHeight(vtkPolyData* data)
{
	size_t nPoints = data->GetNumberOfPoints();
	double min = _colorLookupTable->getMinRange();
	if (min == _colorLookupTable->DEFAULTMINVALUE && nPoints > 0)
	{
		double p[3];
		data->GetPoint(0,p);
		min = p[2];

		for (size_t i=1; i<nPoints; i++)
		{
			data->GetPoint(i,p);
			if (p[2]<min) min = p[2];
		}
	}
	return min;
}

double VtkColorByHeightFilter::getMaxHeight(vtkPolyData* data)
{
	size_t nPoints = data->GetNumberOfPoints();
	double max = _colorLookupTable->getMaxRange();
	if (max == _colorLookupTable->DEFAULTMAXVALUE && nPoints > 0)
	{
		double p[3];
		data->GetPoint(0,p);
		max = p[2];

		for (size_t i=1; i<nPoints; i++)
		{
			data->GetPoint(i,p);
			if (p[2]>max) 
				max = p[2];
		}
	}
	return max;
}


void VtkColorByHeightFilter::SetLimits(double min, double max)
{
	if (min < max)
	{
		this->_colorLookupTable->setRange(min, max);
	}
	else 
		vtkstd::cout << "VtkColorByHeightFilter::SetLimits(min, max) - Limits not changed because min value > max value." << vtkstd::endl;
}

