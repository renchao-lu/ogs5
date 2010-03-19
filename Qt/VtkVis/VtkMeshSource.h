/**
 * \file VtkMeshSource.h
 * 19/03/2010 KR Initial implementation
 *
 */


#ifndef VTKMESHSOURCE_H
#define VTKMESHSOURCE_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>

#include "GEOObjects.h"

namespace Mesh_Group
{
	class CFEMesh;
}

/**
 * VtkStationSource is a VTK source object which can be used to feed a visualization
 * pipeline with point data. As a vtkPolyDataAlgorithm it outputs polygonal data.
 */
class VtkMeshSource : public vtkPolyDataAlgorithm
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkMeshSource* New();

	vtkTypeRevisionMacro(VtkMeshSource,vtkPolyDataAlgorithm);

	/// Sets the stations as a vector
	void setMesh(const Mesh_Group::CFEMesh* mesh) { _mesh = mesh; };

	/// Prints its data on a stream.
	void PrintSelf(ostream& os, vtkIndent indent);

protected:
	VtkMeshSource();
	~VtkMeshSource() {};

	/// Computes the polygonal data object.
	int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	/// The stations to visualize
	const Mesh_Group::CFEMesh* _mesh;

private:


};

#endif // VTKMESHSOURCE_H
