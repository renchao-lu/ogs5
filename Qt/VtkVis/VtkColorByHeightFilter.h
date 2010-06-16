/**
 * \file VtkColorByHeightFilter.h
 * 21/04/2010 KR Initial implementation
 *
 */


#ifndef VTKCOLORBYHEIGHTFILTER_H
#define VTKCOLORBYHEIGHTFILTER_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>

#include "ColorLookupTable.h"

/**
 * \brief VTK filter object for colouring vtkPolyData objects based on z-coordinates.
 *
 * This filter class is basically a container for a ColorLookupTable. In fact, you can get the underlying
 * ColorLookupTable using the method GetColorLookupTable(). Using this method allows the user to set a number
 * of properties on that lookup table such as interpolation method, the range of values over which the lookup
 * table is calculated and so on.
 * If no range boundaries are explicitely set, the minimum and maximum height value will be calculated from 
 * the data and set as minimum and maximum value for the lookup table.
 */
class VtkColorByHeightFilter : public vtkPolyDataAlgorithm
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkColorByHeightFilter* New();

	vtkTypeRevisionMacro(VtkColorByHeightFilter, vtkPolyDataAlgorithm);

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);

	/// Returns the underlying colour look up table object.
	ColorLookupTable* GetColorLookupTable() const { return _colorLookupTable; }

	/// Manually set boundaries for the look-up table.
	void SetLimits(double min, double max);

protected:
	VtkColorByHeightFilter();
	~VtkColorByHeightFilter();

	/// Computes the unstructured grid data object.
	int RequestData(vtkInformation* request, 
		            vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector);


private:
	/// Calculates the color lookup table based on set parameters.
	ColorLookupTable* BuildColorTable();

	/// Returns the value of the smallest z-coordinate in the data set.
	double getMinHeight(vtkPolyData* data);

	/// Returns the value of the largest z-coordinate in the data set.
	double getMaxHeight(vtkPolyData* data);

	ColorLookupTable* _colorLookupTable;
};

#endif // VTKCOLORBYHEIGHTFILTER_H
