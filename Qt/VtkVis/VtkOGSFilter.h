/**
 * \file VtkOGSFilter.h
 * 14/04/2010 KR Initial implementation
 *
 */


#ifndef VTKOGSFILTER_H
#define VTKOGSFILTER_H


#include <vtkPolyDataAlgorithm.h>
#include <vtkUnstructuredGridAlgorithm.h>

/**
 * \brief Convenience class for storing self-defined VTK filters
 */
class VtkOGSFilter
{
public:
	VtkOGSFilter() {};
	~VtkOGSFilter() {};

	static vtkPolyDataAlgorithm* ColorByHeight(vtkUnstructuredGridAlgorithm* algorithm);

	static vtkPolyDataAlgorithm* Point2GlyphFilter(vtkPolyDataAlgorithm* algorithm);

	static vtkPolyDataAlgorithm* Line2CylinderFilter(vtkPolyDataAlgorithm* algorithm);


};

#endif // VTKOGSFILTER_H
