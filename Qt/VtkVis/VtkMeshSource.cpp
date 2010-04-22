/**
 * \file VtkMeshSource.cpp
 * 19/03/2010 KR Initial implementation
 *
 */

// ** VTK INCLUDES **
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkObjectFactory.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>

// OGS Cell Types
#include <vtkTriangle.h>
#include <vtkQuad.h>
#include <vtkHexahedron.h>
#include <vtkTetra.h>
#include <vtkWedge.h> // == Prism
#include <vtkLine.h>

#include "VtkMeshSource.h"

vtkStandardNewMacro(VtkMeshSource);
vtkCxxRevisionMacro(VtkMeshSource, "$Revision$");

VtkMeshSource::VtkMeshSource()
{
	this->SetNumberOfInputPorts(0);
}

void VtkMeshSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_nodes->size() == 0 || _elems->size() == 0)
		return;

	os << indent << "== VtkMeshSource ==" << "\n";

	int i = 0;
	for (std::vector<GEOLIB::Point*>::const_iterator it = _nodes->begin();
		it != _nodes->end(); ++it)
	{
		os << indent << "Point " << i <<" (" << (*it)[0] << ", " << (*it)[1] << ", " << (*it)[2] << ")" << std::endl;
		i++;
	}

	i = 0;
	for (std::vector<GridAdapter::Element*>::const_iterator it = _elems->begin();
		it != _elems->end(); ++it)
	{
		
		os << indent << "Element " << i <<": ";
		for (size_t t=0; t<(*it)->nodes.size(); t++)
			os << (*it)->nodes[t] << " ";
		os << std::endl;
		i++;
	}


}

int VtkMeshSource::RequestData( vtkInformation* request, 
							    vtkInformationVector** inputVector, 
								vtkInformationVector* outputVector )
{
	int scalingFactor = 10;
	size_t nPoints = _nodes->size();
	size_t nElems  = _elems->size();
	size_t nElemNodes = 0;
	if (nPoints == 0 || nElems == 0)
		return 0;

	vtkSmartPointer<vtkPoints> gridPoints = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkInformation> outInfo = outputVector->GetInformationObject(0);
	vtkSmartPointer<vtkUnstructuredGrid> output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	gridPoints->Allocate(_nodes->size());
	output->Allocate(nElems);

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	// Generate mesh nodes
	for (size_t i=0; i<nPoints; i++)
		gridPoints->InsertPoint(i, (*(*_nodes)[i])[0], (*(*_nodes)[i])[1], (*(*_nodes)[i])[2]*scalingFactor);

	// Generate mesh elements
	for (size_t i=0; i<nElems; i++)
	{
		vtkCell* newCell;
		
		if ((*_elems)[i]->type == GridAdapter::TRIANGLE)
			newCell = vtkTriangle::New();
		if ((*_elems)[i]->type == GridAdapter::LINE)
			newCell = vtkLine::New();
		if ((*_elems)[i]->type == GridAdapter::QUAD)
			newCell = vtkQuad::New();
		if ((*_elems)[i]->type == GridAdapter::HEXAHEDRON)
			newCell = vtkHexahedron::New();
		if ((*_elems)[i]->type == GridAdapter::TETRAEDER)
			newCell = vtkTetra::New();
		if ((*_elems)[i]->type == GridAdapter::PRISM)
			newCell = vtkWedge::New();

		nElemNodes = (*_elems)[i]->nodes.size();
		for (size_t j=0; j<nElemNodes; j++)	
			newCell->GetPointIds()->SetId(j, (*_elems)[i]->nodes[j]);

		output->InsertNextCell(newCell->GetCellType(), newCell->GetPointIds());
		newCell->Delete();
	}


	output->SetPoints(gridPoints);	

	this->GetProperties()->SetOpacity(0.5);
	this->GetProperties()->SetEdgeVisibility(1);

	return 1;
}

