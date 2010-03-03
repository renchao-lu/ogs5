/**
 * \file VtkVisPipelineItem.h
 * 17/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINEITEM_H
#define VTKVISPIPELINEITEM_H

// ** INCLUDES **
#include "TreeItem.h"

#include <QList>

class vtkPolyDataAlgorithm;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;

/**
 * VtkVisPipelineItem
 */
class VtkVisPipelineItem : public TreeItem
{
public:
	/// Constructor for a source/filter object.
	VtkVisPipelineItem(vtkRenderer* renderer, vtkPolyDataAlgorithm* algorithm,
		TreeItem* parentItem, vtkPolyData* input,
		const QList<QVariant> data = QList<QVariant>());

	~VtkVisPipelineItem();

	QVariant data(int column) const;
	bool setData(int column, const QVariant &value);

	/// Returns the vtkPolyDataAlgorithm object
	vtkPolyDataAlgorithm* algorithm() const { return _algorithm; }

	/// Returns the actor
	vtkActor* actor() const { return _actor; }

	/// Returns if the VTK object is visible in the visualization.
	bool isVisible() const;

	/// Sets the visibility of the VTK object in the visualization.
	void setVisible(bool visible);

private:
	vtkPolyDataAlgorithm* _algorithm;
	vtkPolyData* _input;
	vtkPolyDataMapper* _mapper;
	vtkActor* _actor;
	vtkRenderer* _renderer;

	void Initialize();
};

#endif // VTKVISPIPELINEITEM_H

