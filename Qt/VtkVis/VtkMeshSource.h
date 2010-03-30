/**
 * \file VtkMeshSource.h
 * 19/03/2010 KR Initial implementation
 *
 */


#ifndef VTKMESHSOURCE_H
#define VTKMESHSOURCE_H

// ** INCLUDES **
#include <vtkUnstructuredGridAlgorithm.h>
#include "VtkAlgorithmProperties.h"
#include "GridAdapter.h"


/**
 * VtkStationSource is a VTK source object which can be used to feed a visualization
 * pipeline with point data.
 */
class VtkMeshSource : public vtkUnstructuredGridAlgorithm, public VtkAlgorithmProperties
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkMeshSource* New();

	vtkTypeRevisionMacro(VtkMeshSource, vtkUnstructuredGridAlgorithm);

	/// Sets the nodes and elements of the mesh that should be visualised
	void setMesh(const std::vector<GEOLIB::Point*> *nodes, const std::vector<GridAdapter::Element*> *elems) 
	{ 
		_nodes = nodes; 
		_elems = elems;
	};

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);

protected:
	VtkMeshSource();
	~VtkMeshSource() {};

	/// Computes the unstructured grid data object.
	int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	const std::vector<GEOLIB::Point*> *_nodes;
	const std::vector<GridAdapter::Element*> *_elems;

private:


};

#endif // VTKMESHSOURCE_H
