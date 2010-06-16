/**
 * \file VtkMeshSource.cpp
 * 19/03/2010 KR Initial implementation
 *
 */


#include "VtkMeshSource.h"
#include "ColorLookupTable.h"

// ** VTK INCLUDES **
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include "vtkObjectFactory.h"
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

// OGS Cell Types
#include <vtkHexahedron.h>
#include <vtkLine.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkWedge.h> // == Prism


vtkStandardNewMacro(VtkMeshSource);
vtkCxxRevisionMacro(VtkMeshSource, "$Revision$");

VtkMeshSource::VtkMeshSource() : _matName("Materials")
{
	this->SetNumberOfInputPorts(0);

	this->SetScalarVisibility(false);
	GEOLIB::Color* c = GEOLIB::getRandomColor();
	vtkProperty* vtkProps = GetProperties();
	vtkProps->SetColor((*c)[0]/255.0,(*c)[1]/255.0,(*c)[2]/255.0);
	vtkProps->SetOpacity(0.5);
	vtkProps->SetEdgeVisibility(1);
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
	size_t nPoints = _nodes->size();
	size_t nElems  = _elems->size();
	size_t nElemNodes = 0;
	if (nPoints == 0 || nElems == 0)
		return 0;

	vtkSmartPointer<vtkInformation> outInfo = outputVector->GetInformationObject(0);
	vtkSmartPointer<vtkUnstructuredGrid> output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
		output->Allocate(nElems);

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	// Insert grid points
	vtkSmartPointer<vtkPoints> gridPoints = vtkSmartPointer<vtkPoints>::New();
		gridPoints->Allocate(_nodes->size());
		// Generate mesh nodes
		for (size_t i=0; i<nPoints; i++)
			gridPoints->InsertPoint(i, (*(*_nodes)[i])[0], (*(*_nodes)[i])[1], (*(*_nodes)[i])[2]);

	// Generate attribute vector for material groups
	vtkSmartPointer<vtkIntArray> materialIDs = vtkSmartPointer<vtkIntArray>::New();
	    materialIDs->SetName("Materials");
		materialIDs->SetNumberOfComponents(1);
		//materialIDs->SetNumberOfTuples(nElems);

	// Generate mesh elements
	for (size_t i=0; i<nElems; i++)
	{
		vtkCell* newCell;
		
		switch ((*_elems)[i]->type)
		{
			case GridAdapter::TRIANGLE:
				newCell = vtkTriangle::New();   break;
			case GridAdapter::LINE:
				newCell = vtkLine::New();       break;
			case GridAdapter::QUAD:
				newCell = vtkQuad::New();       break;
			case GridAdapter::HEXAHEDRON:
				newCell = vtkHexahedron::New(); break;
			case GridAdapter::TETRAEDER:
				newCell = vtkTetra::New();      break;
			case GridAdapter::PRISM:
				newCell = vtkWedge::New();      break;
			default:	// if none of the above can be applied
				return 0;
		}

		materialIDs->InsertNextValue((*_elems)[i]->material);

		nElemNodes = (*_elems)[i]->nodes.size();
		for (size_t j=0; j<nElemNodes; j++)	
			newCell->GetPointIds()->SetId(j, (*_elems)[i]->nodes[j]);

		output->InsertNextCell(newCell->GetCellType(), newCell->GetPointIds());
		newCell->Delete();
	}


	output->SetPoints(gridPoints);	

	output->GetCellData()->AddArray(materialIDs);
	output->GetCellData()->SetActiveAttribute(_matName, vtkDataSetAttributes::SCALARS);

	return 1;
}

void VtkMeshSource::setColorsFromMaterials()
{
	vtkSmartPointer<vtkUnsignedCharArray> matColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
		matColors->SetNumberOfComponents(3);
		matColors->SetName("MatColors");

	vtkSmartPointer<vtkIntArray> materialIDs = vtkIntArray::SafeDownCast(this->GetOutput()->GetCellData()->GetArray("Materials"));
	std::map<size_t, GEOLIB::Color*> colorLookupTable;

	size_t nEntries = materialIDs->GetDataSize();
	for (size_t i=0; i<nEntries; i++)
	{
		size_t id = materialIDs->GetComponent(i,0);
		GEOLIB::Color* c = colorLookupTable[id];
		if (c == NULL)
		{
			c = GEOLIB::getRandomColor();
			colorLookupTable[id] = c;
		}
		unsigned char color[3] = { (*c)[0], (*c)[1], (*c)[2] };
		matColors->InsertNextTupleValue(color);
	}

	for (std::map<size_t, GEOLIB::Color*>::const_iterator it = colorLookupTable.begin();
		it != colorLookupTable.end(); ++it)
		delete it->second;
	colorLookupTable.clear();

	this->GetOutput()->GetCellData()->AddArray(matColors);
	this->GetOutput()->GetCellData()->SetActiveAttribute("MatColors", vtkDataSetAttributes::SCALARS);
}