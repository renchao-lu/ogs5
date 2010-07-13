/**
 * \file VtkInteractorStyle.cpp
 * 21/6/2010 LB Initial implementation
 * 
 * Implementation of VtkInteractorStyle
 */

// ** INCLUDES **
#include "VtkCustomInteractorStyle.h"

#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkProp.h>

#include <string>

vtkStandardNewMacro(VtkCustomInteractorStyle);

VtkCustomInteractorStyle::VtkCustomInteractorStyle()
{

}

void VtkCustomInteractorStyle::OnChar()
{
	switch (Interactor->GetKeyCode())
	{
	case '3':
		std::cout << "The 3 key was pressed." << std::endl;
		break;
	case 'a':
		break;
	default:
		vtkInteractorStyleTrackballCamera::OnChar();
	}
}

void VtkCustomInteractorStyle::highlightActor( vtkActor* actor )
{
	HighlightProp((vtkProp*)actor);
}
