/**
 * \file VtkCustomInteractorStyle.h
 * 21/6/2010 LB Initial implementation
 * 
 */


#ifndef VTKCUSTOMINTERACTORSTYLE_H
#define VTKCUSTOMINTERACTORSTYLE_H

// ** INCLUDES **
#include <QObject>

#include <vtkInteractorStyleTrackballCamera.h>

/**
 * VtkCustomInteractorStyle is sub classed from vtkInteractorStyleTrackballCamera.
 */
class VtkCustomInteractorStyle : public QObject, public vtkInteractorStyleTrackballCamera
{
	Q_OBJECT

public:
	static VtkCustomInteractorStyle* New();
	vtkTypeMacro (VtkCustomInteractorStyle, vtkInteractorStyleTrackballCamera);

	/// Handles key press events.
	virtual void OnChar();

public slots:
	void highlightActor(vtkActor* prop);

protected:
	VtkCustomInteractorStyle();
	

private:

};

#endif // VTKINTERACTORSTYLE_H
