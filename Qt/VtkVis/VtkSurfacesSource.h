/**
 * \file VtkSurfacesSource.h
 * 3/2/2010 LB Initial implementation
 *
 */


#ifndef VTKSURFACESSOURCE_H
#define VTKSURFACESSOURCE_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>
#include "VtkAlgorithmProperties.h"

#include "Surface.h"

/**
* VtkSurfacesSource is a VTK source object which can be used to feed a visualization
* pipeline with surface data. As a vtkPolyDataAlgorithm it outputs polygonal data.
 */
class VtkSurfacesSource : public vtkPolyDataAlgorithm, public VtkAlgorithmProperties
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkSurfacesSource* New();

	vtkTypeRevisionMacro(VtkSurfacesSource,vtkPolyDataAlgorithm);

	/// Sets the surfaces vector
	void setSurfaces(std::vector<GEOLIB::Surface*>& surfaces) { _surfaces = surfaces; };

	/// Prints its data on a stream.
	void PrintSelf(ostream& os, vtkIndent indent);

protected:
	VtkSurfacesSource();
	~VtkSurfacesSource() {};

	/// Computes the polygonal data object.
	int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	/// The surfaces to visualize
	std::vector<GEOLIB::Surface*> _surfaces;

private:

};

#endif // VTKSURFACESSOURCE_H
