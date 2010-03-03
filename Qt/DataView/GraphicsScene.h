/**
 * \file GraphicsScene.h
 * 24/9/2009 LB Initial implementation
 *
 */
#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

// ** INCLUDES **
#include <QGraphicsScene>

#include "Model.h"
#include "Enums.h"

#include <QVector>
#include <QMap>

class TreeModel;
class StationTreeModel;
class QItemSelection;
class QModelIndex;

/**
 * The GraphicsScene is the scene representation for the Q2DGraphicsView.
 * The graphics items data is connected to items in models derived from
 * Model. The method loadAll() should be called to populate the
 * scene with graphics items representing  data structures. The scene
 * also
 */
class GraphicsScene : public QGraphicsScene
{
	Q_OBJECT

public:

	/**
	 * Constructor.
	 * \param parent The parent Qt object
	 * \param mgr The OpenSG scene manager that handles the 3D view
	 * \return
	 */
	GraphicsScene(QObject *parent = 0);
	~GraphicsScene();

	/// Populates the scene with graphics items representing  data structures.
	//void loadAll();

	/// Clears scene first before loading new data
	//void reloadAll();

	/// Loads raster data (images or ArcGIS data) into the scene
	void drawPixmap(const QPixmap &img, const QPointF &origin, const double &scalingFactor);

	int backgroundGridSize() const;

	void setBackgroundGridSize(int size);
	//double viewScaling() const;
	//void setViewScaling(double scaling);

public slots:
	/// Sets the viewplane of the scene
	void setViewPlane(EViewPlane viewplane);

	void togglePntsVisibility(bool visible);
	void toggleLinesVisibility(bool visible);
	void toggleStationsVisibility(bool visible);
	void toggleMshVisibility(bool visible);
	void toggleMshNodeVisibility(bool visible);
	void toggleMshElemVisibility(bool visible);

	/// Selects the graphic items. This should be connected with the signal
	/// itemsSelected from the Model class.
	void selectItems(QVector<QGraphicsItem*> items);

	/// Deselects the graphic items. This should be connected with the signal
	/// itemsDeselected from the Model class.
	void deselectItems(QVector<QGraphicsItem*> items);


	/// Loads all items in flat table based model.
	void loadItemsFromTableModel(Model* model);
	void loadItemsFromTreeModel(StationTreeModel* model, std::string name);

protected:
	/// Draws a checkerboard background.
	void drawBackground( QPainter *painter, const QRectF& rect);

protected slots:
	/// Updates selection changes to visualize selection on 2d and 3d items.
	/// This is called when QGraphicsScene�s signal selectionChanged() is emitted.
	void emitSelectionChanged();

	/// Updates selection changes to visualize selection on 2d and 3d items
	/// from outside (should be connected with DataView itemSelectionChanged() ).
	void selectionChangedFromOutside();

	/// Is called when a 2DPntGraphicsItem is changed by the model
	void updateItems(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
	/// Toggles 2d item visibility on the specified model type.
	void toggleModelItemVisibility(Model::ModelContentType modelType, bool visible);
	void toggleModelItemVisibilityRecursive( QModelIndex parent, Model* model, bool visible );

	int _backgroundGridSize;
	EViewPlane _viewplane;
	double _viewScaling;
	bool _isPanning;

	/// A QVector container for storing Models
	//QVector<Model*> _models;

	/// Model acces by ModelContentType
	QMap<Model::ModelContentType, Model*> _modelsByType;

	/// The old item selection. Is set in emitSelectionChanged()
	QItemSelection _oldSelection;

signals:
	void sceneViewPanned(double x, double y);

	/// Is emitted on changing the selection of items in the scene, e.g.
	/// through clicking on an item in the graphics view.
	void itemSelectionChanged(const QItemSelection & selected,
		const QItemSelection & deselected);

	/// Is emitted when items are loaded
	void sceneChanged();
};

#endif // GRAPHICSSCENE_H
