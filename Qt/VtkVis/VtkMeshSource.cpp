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
#include <vtkTriangle.h>
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
	//vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	//vtkPoints* newPoints = vtkPoints::New();
	vtkCellArray* newVerts = vtkCellArray::New();
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::New();

	//newPoints->Allocate(nPoints);
	//newVerts->Allocate(nStations);

	size_t currentIndex = 0;

	if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
		return 1;

	int lastMaxIndex = 0;

/*
	// Generate point objects
	for (std::vector<GEOLIB::Point*>::const_iterator it = _mesh->nod_vector->begin();
		it != _mesh->nod_vector->end(); ++it)
	{
		const double* coords = (*it)->getData();
		vtkIdType sid[1];
		sid[0] = newPoints->InsertNextPoint(coords);
		newVerts->InsertNextCell(1, sid);
	}
*/
	// begin minimal example:
	vtkPoints* trianglePoints = vtkPoints::New();
trianglePoints->SetNumberOfPoints(3);
trianglePoints->InsertPoint(0, 4408077, 5719585, 0.000000);
trianglePoints->InsertPoint(1, 4481596, 5781261, 0.000000);
trianglePoints->InsertPoint(2, 4481596, 5719585, 0.000000);
vtkTriangle* aTriangle = vtkTriangle::New();
aTriangle->GetPointIds()->SetId(0, 0);
aTriangle->GetPointIds()->SetId(1, 1);
aTriangle->GetPointIds()->SetId(2, 2);
output->Allocate(1, 1);
output->InsertNextCell(aTriangle->GetCellType(), aTriangle->GetPointIds());
output->SetPoints(trianglePoints);
/*
	vtkTriangle* triangle = vtkTriangle::New(); // HACK for triangle meshes
	const double pointa[3] = {4408077, 5719585, 0.000000};
	const double pointb[3] = {4481596, 5781261, 0.000000};
	const double pointc[3] = {4481596, 5719585, 0.000000};
	vtkIdList* idList = vtkIdList::New();
	idList->InsertNextId(0);
	idList->InsertNextId(1);
	idList->InsertNextId(2);

	output->InsertNextCell(triangle->GetCellType(), idList);
	output->SetPoints(newPoints);
	//output->SetVerts(newVerts);
*/
	// end minimal example

/*
	vtkTriangle* triangle = vtkTriangle::New(); // HACK for triangle meshes
	// Generate mesh elements
	for (std::vector<Mesh_Group::CElem*>::const_iterator it = _mesh->ele_vector.begin();
		it != _mesh->ele_vector.end(); ++it)
	{
		vtkIdList* idList = vtkIdList::New();
		for (size_t id=0; id<3; id++)  // HACK for triangle meshes
		{
			Mesh_Group::CNode* node = (*it)->GetNode(id);
			const double point[3] = {node->X(), node->Y(), node->Z()};
			vtkIdType sid[1];
			sid[0] = newPoints->InsertNextPoint(point);
			idList->InsertNextId(currentIndex);
			currentIndex++;
		}
		output->InsertNextCell(triangle->GetCellType(), idList);
	}
	
	output->SetPoints(newPoints);

*/
	
	//output->SetPoints(newPoints);
	//newPoints->Delete();

//	output->SetVerts(newVerts);
//	newVerts->Delete();

	return 1;
}

int VtkMeshSource::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

	return 1;
}
