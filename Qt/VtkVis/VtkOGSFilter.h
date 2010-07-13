/**
 * \file VtkOGSFilter.h
 * 14/04/2010 KR Initial implementation
 *
 */

#ifndef VTKOGSFILTER_H
#define VTKOGSFILTER_H

#include <string>
#include <vector>
#include <cstddef>

class vtkAlgorithm;
class vtkPolyDataAlgorithm;
class vtkUnstructuredGridAlgorithm;

class VtkFilterItem;


/**
 * \brief Access function for use of VTK filters in OGS.
 *
 * Filter functions are defined static and can thus be used from anywhere without object instantiation. For controlled
 * use in the GUI filters need to be "registered" in the Constructor and can then be accesses via the apply() function.
 */
class VtkOGSFilter
{
public:
	/// Registered filter aliases
	enum OGSVisFilter
	{
		POINTTOGLYPHFILTER       = 0,
		LINETOCYLINDERFILTER     = 1,
		SURFACEFILTER            = 2,
		COLORBYHEIGHTFILTER_GRID = 3,
		COLORBYHEIGHTFILTER_POLY = 4,
		MATGROUPFILTER           = 5,
		TEXTOGRIDFILTER          = 6,
		TEXTOSURFACEFILTER       = 7,
		THRESHOLDINGFILTER       = 8
	};

	/// Constructor
	VtkOGSFilter();
	~VtkOGSFilter();

	/// Access function for applying filters via the GUI
	vtkAlgorithm* apply(vtkAlgorithm* input, VtkOGSFilter::OGSVisFilter filter);

	/// Returns all registered filters as VtkFilterItem-Objects.
	std::vector<VtkFilterItem> getAvailableFilters() const { return _availableFilters; };

	/**
	 * \brief Elevation filter for colouring objects based on their height. 
	 * 
	 * This function uses the ColorLookupTable class and allows customisaton of the applied transfer function.
	 */
	static vtkPolyDataAlgorithm* ColorByHeight(vtkPolyDataAlgorithm* algorithm);

	/**
	 * \brief Elevation filter for colouring objects based on their height. 
	 * 
	 * This function uses the ColorLookupTable class and allows customisaton of the applied transfer function.
	 */
	static vtkPolyDataAlgorithm* ColorByHeight(vtkUnstructuredGridAlgorithm* algorithm);

	/// Filter for generating cylinders from lines. Currently not working correctly.
	static vtkPolyDataAlgorithm* Line2CylinderFilter(vtkPolyDataAlgorithm* algorithm, double radius = 50);

	/// Colorises the mesh based on its material groups.
	static vtkUnstructuredGridAlgorithm* MaterialGroupFilter(vtkUnstructuredGridAlgorithm* algorithm);

	/// Filter for generating spheres from points.
	static vtkPolyDataAlgorithm* Point2GlyphFilter(vtkPolyDataAlgorithm* algorithm, double radius = 150);

	/// Filter for generating a surface (i.e. a polydata-object) from a mesh (i.e. a grid-object)
	/// This is a base-filter for various other filters that require the input to be of type PolyData.
	static vtkPolyDataAlgorithm* SurfaceFilter(vtkUnstructuredGridAlgorithm* algorithm);

	/// Maps a texture onto a grid.
	static vtkPolyDataAlgorithm* TextureToGridFilter(vtkUnstructuredGridAlgorithm* algorithm);

	/// Maps a texture onto a surface.
	static vtkPolyDataAlgorithm* TextureToSurfaceFilter(vtkPolyDataAlgorithm* algorithm);

	/// Filtering data based on given thresholds. Currently not working correctly.
	static vtkUnstructuredGridAlgorithm* ThresholdFilter(vtkUnstructuredGridAlgorithm* algorithm, size_t min, size_t max);

private:
	std::vector<VtkFilterItem> _availableFilters;

	static float normalize(float min, float max, float val) { return ((val-min)/static_cast<float>(max-min)); };

};

/**
 * \brief Stores information about filters in VtkOGSFilter for access-routines from the GUI.
 */
class VtkFilterItem
{
public:
	/// Stores on which objects a filter can be applied. 
	enum VtkTargetObject
	{
		POLYDATA         = 0,
		UNSTRUCTUREDGRID = 1
	};

	/**
	 * Constructor
	 * \param name The name of the filter that will be displayed in the GUI
	 * \param filter The registered name of the filter in VtkOGSFilter
	 * \param target The object type on which the filter can be applied 
	 */
	VtkFilterItem(std::string name, VtkOGSFilter::OGSVisFilter filter, VtkTargetObject target = VtkFilterItem::POLYDATA);

	/// Returns the name or description of the filter.
	const std::string& name() const { return _name; };

	/// Returns the registered name for use in VtkOGSFilter.
	const VtkOGSFilter::OGSVisFilter& filter() const { return _filter; };

	/// Returns the object type on which the filter can be applied.
	const VtkTargetObject& target() const { return _target; };

private:
	std::string _name;
	VtkOGSFilter::OGSVisFilter _filter;
	VtkTargetObject _target;
};



#endif // VTKOGSFILTER_H
