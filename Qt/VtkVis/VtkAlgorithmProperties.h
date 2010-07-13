/**
 * \file VtkAlgoritmProperties.h
 * 24/03/2010 KR Initial implementation
 *
 */


#ifndef VTKALGORITHMPROPERTIES_H
#define VTKALGORITHMPROPERTIES_H

// ** INCLUDES **
#include <vtkProperty.h>
#include <vtkTexture.h>

/**
 * \brief Contains properties for the visualization of objects as VtkVisPipelineItems
 */
class VtkAlgorithmProperties 
{

public:
	/// Returns the properties
	vtkProperty* GetProperties() const { return _property; };
	/// Returns a texture (if one has been assigned).
	vtkTexture* GetTexture()       { return _texture; };
	/// Sets a texture for the VtkVisPipelineItem.
	void SetTexture(vtkTexture* t) { _texture = t; };
	bool GetScalarVisibility() const { return _scalarVisibility; }
	void SetScalarVisibility(bool on) { _scalarVisibility = on; }

protected:
	/// Constructor (sets default values)
	VtkAlgorithmProperties() 
	{ 
		_property = vtkProperty::New(); 
		_texture  = NULL;
		_scalarVisibility = true;
	}

	~VtkAlgorithmProperties() 
	{
		_property->Delete();
		if (_texture != NULL) _texture->Delete();
	};

private:
	// Properties set on vtkActor
	vtkProperty* _property;
	vtkTexture* _texture;

	// Properties set on vtkMapper
	bool _scalarVisibility;

};

#endif // VTKALGORITHMPROPERTIES_H
