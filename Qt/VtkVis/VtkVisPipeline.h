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

class vtkPolyDataAlgorithm;
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
	void addPipelineItem(vtkPolyDataAlgorithm* source, 
		QModelIndex parent = QModelIndex(), vtkPointSet* input = NULL);

	void removeSourceItem(Model* model);
	void removeSourceItem(StationTreeModel* model, const std::string &name);
	void removePipelineItem(QModelIndex index);

private:
	vtkRenderer* _renderer;
	QVector<vtkPolyDataAlgorithm*> _sources;

signals:
	void vtkVisPipelineChanged();

};

#endif // VTKVISPIPELINE_H
