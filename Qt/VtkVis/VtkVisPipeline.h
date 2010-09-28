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
#include <QMap>

class vtkAlgorithm;
class vtkLight;
class vtkPointSet;
class vtkRenderer;
class vtkActor;
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

	/// \brief Constructor
	VtkVisPipeline(vtkRenderer* renderer, QObject* parent = 0);

	/// \brief Emits vtkVisPipelineChanged() and calls base class method.
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	/// \brief Adds a light to the scene at the given coordinates.
	void addLight(const GEOLIB::Point &pos);

	/// \brief Returns a light (or NULL) for the given coordinates.
	vtkLight* getLight(const GEOLIB::Point &pos) const;

	/// \brief Removes a light at the given coordinates (if possible).
	void removeLight(const GEOLIB::Point &pos);

	/// \brief Sets the background-colour of the scene.
	void setBGColor(const GEOLIB::Color &color);

	/// \brief Returns the QModelIndex of VtkVisPipelineItem which actor
	/// is the given one.
	QModelIndex getIndex(vtkActor* actor);

	Qt::ItemFlags flags( const QModelIndex &index ) const;

public slots:
	/// \brief Adds the given Model to the pipeline.
	void addPipelineItem(Model* model);
	void addPipelineItem(MshModel* model, const QModelIndex &idx);
	void addPipelineItem(StationTreeModel* model, const std::string &name);

	/// \brief Inserts the vtkAlgorithm as a child of the given QModelIndex to the pipeline.
	void addPipelineItem(vtkAlgorithm* source, QModelIndex parent = QModelIndex());

	/// \brief Removes the given Model (and all attached vtkAlgorithms) from the pipeline.
	void removeSourceItem(Model* model);
	void removeSourceItem(MshModel* model, const QModelIndex &idx);
	void removeSourceItem(StationTreeModel* model, const std::string &name);

	/// \brief Removes the vtkAlgorithm at the given QModelIndex (and all attached
	/// vtkAlgorithms) from the pipeline.
	void removePipelineItem(QModelIndex index);

private:
	vtkRenderer* _renderer;
	QVector<vtkAlgorithm*> _sources;
	std::list<vtkLight*> _lights;
	QMap<vtkActor*, QModelIndex> _actorMap;


signals:
	/// \brief Is emitted when a pipeline item was added or removed.
	void vtkVisPipelineChanged();

};

#endif // VTKVISPIPELINE_H
