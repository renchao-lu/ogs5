/**
 * \file VtkStationSource.h
 * 24/02/2010 KR Initial implementation
 *
 */


#ifndef VTKSTATIONSOURCE_H
#define VTKSTATIONSOURCE_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>

#include "GEOObjects.h"

/**
 * VtkStationSource is a VTK source object which can be used to feed a visualization
 * pipeline with point data. As a vtkPolyDataAlgorithm it outputs polygonal data.
 */
class VtkStationSource : public vtkPolyDataAlgorithm
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkStationSource* New();

	vtkTypeRevisionMacro(VtkStationSource,vtkPolyDataAlgorithm);

	/// Sets the stations as a vector
	void setStations(const std::vector<GEOLIB::Point*> *stations) { _stations = stations; };

	/// Prints its data on a stream.
	void PrintSelf(ostream& os, vtkIndent indent);

protected:
	VtkStationSource();
	~VtkStationSource() {};

	/// Computes the polygonal data object.
	int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

	/// The stations to visualize
	const std::vector<GEOLIB::Point*> *_stations;

private:


};

#endif // VTKSTATIONSOURCE_H
