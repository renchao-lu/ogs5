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
 * \brief VTK source object for the visualisation of unstructured grids
 */
class VtkMeshSource : public vtkUnstructuredGridAlgorithm, public VtkAlgorithmProperties
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkMeshSource* New();

	vtkTypeRevisionMacro(VtkMeshSource, vtkUnstructuredGridAlgorithm);

	const char* GetMaterialArrayName() const { return _matName; }

	/// Sets the nodes and elements of the mesh that should be visualised
	void setMesh(const std::vector<GEOLIB::Point*> *nodes, const std::vector<GridAdapter::Element*> *elems) 
	{ 
		_nodes = nodes; 
		_elems = elems;
	};

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);

	/** 
	 * \brief Generates random colors based on the material scalar value. 
	 * Each element of the mesh is assigned an RGB-value based on its material group.
	 * This method should only be called after setMesh()!
	 */
	void setColorsFromMaterials();

protected:
	VtkMeshSource();
	~VtkMeshSource() {};

	/// Computes the unstructured grid data object.
	int RequestData(vtkInformation* request, 
		            vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector);



	const std::vector<GEOLIB::Point*> *_nodes;
	const std::vector<GridAdapter::Element*> *_elems;

private:
	const char* _matName;

};

#endif // VTKMESHSOURCE_H
