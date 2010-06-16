/**
 * \file VtkVisPipeline.h
 * 17/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINE_H
#define VTKVISPIPELINE_H

// ** INCLUDES **
#include "TreeModel.h"
#include "Color.h"
#include "Point.h"
#include <QVector>

#include "VtkVisPipelineItem.h"

class vtkAlgorithm;
class vtkLight;
class vtkPointSet;
class vtkRenderer;
class Model;
class TreeModel;
class MshModel;
class StationTreeModel;
class QModelIndex;


/**
 * \brief VtkVisPipeline manages the VTK visualization.
 * It is a TreeModel and provides functions for adding and removing OGS
 * Model or vtkAlgorithm objects.
 */
class VtkVisPipeline : public TreeModel
{
	Q_OBJECT

public:

	/// Constructor
	VtkVisPipeline(vtkRenderer* renderer, QObject* parent = 0);

	/// Adds a light to the scene at the given coordinates.
	void addLight(const GEOLIB::Point &pos);

	/// Returns a light (or NULL) for the given coordinates.
	vtkLight* getLight(const GEOLIB::Point &pos) const;

	/// Removes a light at the given coordinates (if possible).
	void removeLight(const GEOLIB::Point &pos);

	/// Sets the background-colour of the scene.
	void setBGColor(const GEOLIB::Color &color);


	Qt::ItemFlags flags( const QModelIndex &index ) const;

public slots:
	/// Adds the given Model to the pipeline.
	void addPipelineItem(Model* model);
	void addPipelineItem(MshModel* model, const QModelIndex &idx);
	void addPipelineItem(StationTreeModel* model, const std::string &name);

	/// Inserts the vtkAlgorithm as a child of the given QModelIndex to the pipeline.
	void addPipelineItem(vtkAlgorithm* source, QModelIndex parent = QModelIndex());

	/// Removes the given Model (and all attached vtkAlgorithms) from the pipeline.
	void removeSourceItem(Model* model);
	void removeSourceItem(MshModel* model, const QModelIndex &idx);
	void removeSourceItem(StationTreeModel* model, const std::string &name);

	/// Removes the vtkAlgorithm at the given QModelIndex (and all attached
	/// vtkAlgorithms) from the pipeline.
	void removePipelineItem(QModelIndex index);

private:
	vtkRenderer* _renderer;
	QVector<vtkAlgorithm*> _sources;
	std::list<vtkLight*> _lights;


signals:
	/// Is emitted when a pipeline item was added or removed.
	void vtkVisPipelineChanged();

};

#endif // VTKVISPIPELINE_H
