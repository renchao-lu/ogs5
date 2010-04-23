/**
 * \file VtkColorByHeightFilter.h
 * 21/04/2010 KR Initial implementation
 *
 */


#ifndef VTKCOLORBYHEIGHTFILTER_H
#define VTKCOLORBYHEIGHTFILTER_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>

class ColorLookupTable;

/**
 * \brief VTK filter object for colouring vtkPolyData objects based on z-coordinates.
 *
 * The lower and upper boundaries for colouring surfaces can be set manually using the SetLimits() methods.
 * If no boundaries are set they are calculated from the data.
 */
class VtkColorByHeightFilter : public vtkPolyDataAlgorithm
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkColorByHeightFilter* New();

	vtkTypeRevisionMacro(VtkColorByHeightFilter, vtkPolyDataAlgorithm);

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);

	/// Manually set boundaries for the look-up table.
	void SetLimits(double min, double max);

protected:
	VtkColorByHeightFilter();
	~VtkColorByHeightFilter() {};

	/// Computes the unstructured grid data object.
	int RequestData(vtkInformation* request, 
		            vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector);

private:
	static const int DEFAULTMINVALUE=-9999;
	static const int DEFAULTMAXVALUE=9999;

	ColorLookupTable* BuildColorTable(double min, double max);

	/// Returns the value of the smallest z-coordinate in the data set.
	double getMinHeight(vtkPolyData* data);

	/// Returns the value of the largest z-coordinate in the data set.
	double getMaxHeight(vtkPolyData* data);

	double _min, _max;


};

#endif // VTKCOLORBYHEIGHTFILTER_H
