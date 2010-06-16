/**
 * \file VtkOGSPolyDataAlgorithm.h
 * 28/05/2010 KR initial implementation
 *
 */


#ifndef VTKOGSPOLYDATAALGORITHM_H
#define VTKOGSPOLYDATAALGORITHM_H

// ** INCLUDES **
#include <vtkPolyDataAlgorithm.h>
#include <vtkImageData.h>
#include "VtkAlgorithmProperties.h"

class QImage;

/**
 * \brief Container class to keep properties linked to a vtk object
 */
class VtkTextureOnSurfaceFilter : public vtkPolyDataAlgorithm, public VtkAlgorithmProperties
{
public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkTextureOnSurfaceFilter* New();

	vtkTypeRevisionMacro(VtkTextureOnSurfaceFilter,vtkPolyDataAlgorithm);

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);	

	/// Sets the raster/image to be used as a texture map
	void SetRaster(QImage &img, std::pair<float, float> origin, double scalingFactor);

	/// Converts a QImage-object into a vtkTexture-object
	static vtkTexture* ConvertImageToTexture(QImage &img);

protected:
	VtkTextureOnSurfaceFilter();
	~VtkTextureOnSurfaceFilter();

	/// Copies the input data.
	int RequestData(vtkInformation* request, 
		            vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector);

private:
	static float normalize(float min, float max, float val) { return ((val-min)/static_cast<float>(max-min)); };

	std::pair<float, float> _origin;
	double _scalingFactor;
};

#endif // VTKOGSPOLYDATAALGORITHM_H
