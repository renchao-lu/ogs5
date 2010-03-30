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
#include <vtkTriangle.h>

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

int VtkMeshSource::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	size_t nPoints = _nodes->size();
	size_t nElems  = _elems->size();
	size_t nElemNodes = 0;
	if (nPoints == 0 || nElems == 0)
		return 0;

	vtkPoints *gridPoints = vtkPoints::New();
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	gridPoints->Allocate(_nodes->size());
	output->Allocate(nElems);

	size_t cid = 0;

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	for (size_t i=0; i<nPoints; i++)
		gridPoints->InsertPoint(i, (*(*_nodes)[i])[0], (*(*_nodes)[i])[1], (*(*_nodes)[i])[2]);

	// Generate mesh elements
	for (size_t i=0; i<nElems; i++)
	{
		vtkTriangle* triangle = vtkTriangle::New(); // HACK for triangle meshes

		nElemNodes = (*_elems)[i]->nodes.size();
		for (size_t j=0; j<nElemNodes; j++)	
			triangle->GetPointIds()->SetId(j, (*_elems)[i]->nodes[j]);

		output->InsertNextCell(triangle->GetCellType(), triangle->GetPointIds());
	}
	
	output->SetPoints(gridPoints);
	gridPoints->Delete();

	this->GetProperties()->SetOpacity(0.5);
	this->GetProperties()->SetEdgeVisibility(1);

	return 1;
}

/// Create 3d mesh object
/*
int VtkMeshSource::RequestData1( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	if (!_mesh)
		return 0;
	int nPoints = _mesh->nod_vector.size();
	int nElems  = _mesh->ele_vector.size();
	if (nPoints == 0 || nElems == 0)
		return 0;

	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkPoints* gridPoints = vtkPoints::New();
	vtkCellArray* newVerts = vtkCellArray::New();

	//newPoints->Allocate(nPoints);

	size_t cid = 0;

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	output->Allocate(nElems);
	// Generate mesh elements
	for (std::vector<Mesh_Group::CElem*>::const_iterator it = _mesh->ele_vector.begin();
		it != _mesh->ele_vector.end(); ++it)
	{
		vtkTriangle* triangle = vtkTriangle::New(); // HACK for triangle meshes
		for (size_t id=0; id<3; id++)  // HACK for triangle meshes
		{
			Mesh_Group::CNode* node = (*it)->GetNode(id);
			gridPoints->InsertPoint(cid, node->X(), node->Y(), node->Z());
			triangle->GetPointIds()->SetId(id, cid);
			cid++;
		}
		output->InsertNextCell(triangle->GetCellType(), triangle->GetPointIds());
	}
	
	output->SetPoints(gridPoints);


	gridPoints->Delete();

	this->GetProperties()->SetOpacity(0.5);
	this->GetProperties()->SetEdgeVisibility(1);
*/
/*
	vtkPoints* trianglePoints = vtkPoints::New();
	trianglePoints->SetNumberOfPoints(3);
	trianglePoints->InsertPoint(0, 0, 0, 0);
	trianglePoints->InsertPoint(1, 1, 0, 0);
	trianglePoints->InsertPoint(2, .5, .5, 0);
	vtkFloatArray* triangleTCoords = vtkFloatArray::New();
	triangleTCoords->SetNumberOfComponents(3);
	triangleTCoords->SetNumberOfTuples(3);
	triangleTCoords->InsertTuple3(0, 1, 1, 1);
	triangleTCoords->InsertTuple3(1, 2, 2, 2);
	triangleTCoords->InsertTuple3(2, 3, 3, 3);
	vtkTriangle* aTriangle = vtkTriangle::New();
	aTriangle->GetPointIds()->SetId(0, 0);
	aTriangle->GetPointIds()->SetId(1, 1);
	aTriangle->GetPointIds()->SetId(2, 2);
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
	output->Allocate(1, 1);
	output->InsertNextCell(aTriangle->GetCellType(), aTriangle->GetPointIds());
	output->SetPoints(trianglePoints);
	output->GetPointData()->SetTCoords(triangleTCoords);
*/
//	return 1;
//}

int VtkMeshSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
