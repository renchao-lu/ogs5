/**
 * \file VtkVisPipeline.h
 * 17/2/2010 LB Initial implementation
 * 
 */


#ifndef VTKVISPIPELINE_H
#define VTKVISPIPELINE_H

// ** INCLUDES **
#include "TreeModel.h"
#include <QVector>

#include "VtkVisPipelineItem.h"

//class vtkPolyDataAlgorithm;
class vtkAlgorithm;
class vtkPointSet;
class vtkRenderer;
class Model;
class QModelIndex;
class StationTreeModel;

/**
 * VtkVisPipeline
 */
class VtkVisPipeline : public TreeModel
{
	Q_OBJECT

public:
	VtkVisPipeline(vtkRenderer* renderer, QObject* parent = 0);

	//QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	
public slots:
	void addPipelineItem(Model* model);
	void addPipelineItem(StationTreeModel* model, const std::string &name);
	void addPipelineItem(vtkAlgorithm* source, QModelIndex parent = QModelIndex());

	void removeSourceItem(Model* model);
	void removeSourceItem(StationTreeModel* model, const std::string &name);
	void removePipelineItem(QModelIndex index);

private:
	vtkRenderer* _renderer;
	QVector<vtkAlgorithm*> _sources;

signals:
	void vtkVisPipelineChanged();

};

#endif // VTKVISPIPELINE_H
