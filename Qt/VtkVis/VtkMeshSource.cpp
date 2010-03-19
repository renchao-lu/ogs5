/**
 * \file VtkMeshSource.cpp
 * 19/03/2010 KR Initial implementation
 *
 */

#include "msh_mesh.h"

// ** VTK INCLUDES **
#include "VtkMeshSource.h"

#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkObjectFactory.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>

vtkStandardNewMacro(VtkMeshSource);
vtkCxxRevisionMacro(VtkMeshSource, "$Revision$");

VtkMeshSource::VtkMeshSource()
: _mesh(NULL)
{
	this->SetNumberOfInputPorts(0);
}

void VtkMeshSource::PrintSelf( ostream& os, vtkIndent indent )
{
	this->Superclass::PrintSelf(os,indent);

	if (_mesh->ele_vector.size() == 0) // HACK use of ele_vector.size() is probably not correct.
		return;

	os << indent << "== VtkStationSource ==" << "\n";

/*  TODO
	int i = 0;
	for (std::vector<GEOLIB::Point*>::const_iterator it = _stations->begin();
		it != _stations->end(); ++it)
	{
		const double* coords = (*it)->getData();
		os << indent << "Station " << i <<" (" << coords[0] << ", " << coords[1] << ", " << coords[2] << ")\n";
		i++;
	}
*/
}


/// Create 3d mesh object
int VtkMeshSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	if (!_mesh)
		return 0;
	int nPoints = _mesh->nod_vector.size();
	int nElems  = _mesh->ele_vector.size();
	if (nPoints == 0 || nElems == 0)
		return 0;

	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkPoints* newPoints = vtkPoints::New();
	//vtkCellArray* newVerts = vtkCellArray::New();
	vtkUnstructuredGrid* meshGrid = vtkUnstructuredGrid::New();

	newPoints->Allocate(nPoints);
	//newVerts->Allocate(nStations);
/*
	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	int lastMaxIndex = 0;


	// Generate point objects
	for (std::vector<GEOLIB::Point*>::const_iterator it = _mesh->nod_vector->begin();
		it != _mesh->nod_vector->end(); ++it)
	{
		const double* coords = (*it)->getData();
		vtkIdType sid[1];
		sid[0] = newPoints->InsertNextPoint(coords);
		newVerts->InsertNextCell(1, sid);
	}

	// Generate mesh elements
	for (std::vector<GEOLIB::Point*>::const_iterator it = _mesh->ele_vector->begin();
		it != _mesh->ele_vector->end(); ++it)
	{
		vtkIdList* idList = vtkIdList::New()
		for (size_t id=0; id<it->nodes_index.size(); id++) 
		{
			it->GetNode(id);
			const double node[3] = {node->X(), node->Y(), node->Z()};
			vtkIdType sid[1];
			sid[0] = newPoints->InsertNextPoint(node);

			idList->InsertNextId(it->nodes_index[id]);
		}
		meshGrid->InsertNextCell( , idList);
	}
	
	meshGrid->SetPoints(newPoints);



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
*/
	return 1;
}

int VtkMeshSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
