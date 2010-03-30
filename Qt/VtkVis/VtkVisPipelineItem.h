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

class vtkAlgorithm;
class vtkPointSet;
class vtkDataSetMapper;
class vtkActor;
class vtkRenderer;

/**
 * VtkVisPipelineItem
 */
class VtkVisPipelineItem : public TreeItem
{
public:
	/// Constructor for a source/filter object.
	VtkVisPipelineItem(vtkRenderer* renderer, vtkAlgorithm* algorithm,
		TreeItem* parentItem, vtkPointSet* input,
		const QList<QVariant> data = QList<QVariant>());

	~VtkVisPipelineItem();

	QVariant data(int column) const;
	bool setData(int column, const QVariant &value);

	/// Returns the algorithm object
	vtkAlgorithm* algorithm() const { return _algorithm; }

	/// Returns the actor
	vtkActor* actor() const { return _actor; }

	/// Returns if the VTK object is visible in the visualization.
	bool isVisible() const;

	/// Sets the visibility of the VTK object in the visualization.
	void setVisible(bool visible);

private:
	vtkAlgorithm* _algorithm;
	vtkPointSet* _input;
	vtkDataSetMapper* _mapper;
	vtkActor* _actor;
	vtkRenderer* _renderer;

	void Initialize();
};

#endif // VTKVISPIPELINEITEM_H

