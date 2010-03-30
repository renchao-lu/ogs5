/**
 * \file VtkAlgoritmProperties.h
 * 24/03/2010 KR Initial implementation
 *
 */


#ifndef VTKALGORITHMPROPERTIES_H
#define VTKALGORITHMPROPERTIES_H

// ** INCLUDES **
#include <vtkProperty.h>


/**
 * /brief Contains properties for the visualisation of objects as VtkVisPipelineItems
 */
class VtkAlgorithmProperties 
{

public:
	/// Returns the properties
	vtkProperty* GetProperties() { return _property; }

protected:
	VtkAlgorithmProperties() { _property = vtkProperty::New(); }
	~VtkAlgorithmProperties() {};

private:
	vtkProperty* _property;

};

#endif // VTKALGORITHMPROPERTIES_H
