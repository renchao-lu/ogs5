/**
 * \file Model.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef MODEL_H
#define MODEL_H

// ** INCLUDES **
#include <QAbstractListModel>
#include <QVector>
#include <QItemSelection>

class ModelItem;
class QModelIndex;
class QGraphicsItem;
class vtkAlgorithm;

/**
 * The Model is a base class for model implementation representing
 *  data. Data is stored in _data .
 */
class Model : public QAbstractTableModel
{
	Q_OBJECT

public:

	/// Basic types of models (table or tree based)
	enum ModelType
	{
		TABLE_MODEL,
		TREE_MODEL
	};

	/// The derived class type of this model
	enum ModelContentType
	{
		EMPTY_MODEL,
		PNTS_MODEL,
		LINES_MODEL,
		SURFACE_MODEL,
		MSH_MODEL,
		MSH_NODE_MODEL,
		MSH_ELEM_MODEL,
		STATION_MODEL
	};

	/// Constructor
	Model(QString name, QObject* parent = 0);

	/// Virtual empty destructor because this is a polymorphic base class
	virtual ~Model();

	/// Creates model indexes which store a ModelItem as an internal pointer
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	/// Returns the number of rows
	int rowCount(const QModelIndex& parent = QModelIndex()) const;

	/**
	 * Returns flags that the items are editable by default. Overwrite this
	 * function if want non editable items.
	 */
	Qt::ItemFlags flags(const QModelIndex& index) const;

	/// Removes count rows starting at row. Eventually this must be extended
	/// in a derived class to delete wrapped data, see PntsModel::removeRows()
	/// for an example.
	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex() );

	/// Returns a ModelItem from an QModelIndex.
	virtual ModelItem* itemFromIndex(const QModelIndex& index) const;

	/// Returns QModelIndexes from ModelItem.
	QModelIndex indexFromItem(const ModelItem* item) const;

	/// Returns the model type
	ModelType modelType() const;

	/// Returns the model content type
	ModelContentType modelContentType() const;

	/// Returns the dependent submodels
	QVector<Model*> subModels() const;

	/// Returns the name of the model.
	QString name() const { return _name; }
	//void setName(QString val) { _name = val; }

	/// Returns the Vtk polydata source object
	vtkAlgorithm* vtkSource() const { return _vtkSource; }

public slots:
	/// Deletes all data.
	void clearData();

	/// Deletes the selected data.
	void clearSelectedData();

	/// Reloads all data
	virtual void updateData();

	/// Sets the selection on graphics items without emitting signals.
	void setSelectionFromOutside(const QItemSelection & selected, const QItemSelection & deselected);


protected:
	/// On a table model the model items are stored here.
	QVector<ModelItem*> _data;
	
	/// Is this model a flat table model or is it a tree model.
	ModelType _modelType;

	/// Which data represents the model
	ModelContentType _modelContentType;

	/// Models which are dependent and created/managed by this model.
	QVector<Model*> _subModels;

	/// The actually selected items. This is used for removing the actually
	/// selected rows when calling clearSelectedData().
	QItemSelection _selectedItems;

	/// The name of the model. Usually this is the filename where the data
	/// comes from.
	QString _name;

	/// The Vtk data source object. This is the starting point for a Vtk data
	/// visualization pipeline.
	vtkAlgorithm* _vtkSource;

protected slots:

	/// Sets selection on the 2d and 3d graphic items and emits signals
	/// which items are selected and deselected
	virtual void setSelection(const QItemSelection & selected, const QItemSelection & deselected);
	void setSelection(const QItemSelection & selected);

private:
	/// Is called from setSelection() and setSelectionFromOutside. Sets
	/// _selectedItems and returns the selected and deselected items as QVector.
	void updateSelection( const QItemSelection &selected, QVector<QGraphicsItem*> &selectedItems, const QItemSelection &deselected, QVector<QGraphicsItem*> &deselectedItems );
signals:
	void subModelDataChanged(Model* submodel, const QModelIndex &topLeft,
		const QModelIndex &bottomRight);
	
	/// Is emitted when the selection of the model is cleared due to removing items
	void selectionCleared();

	/// Is emitted when items are selected within the model (in setSelection() ).
	void itemsSelected(QVector<QGraphicsItem*> item);

	/// Is emitted when items are deselected within the model (in setSelection() ).
	void itemsDeselected(QVector<QGraphicsItem*> item);

	/// Is emitted when items are selected within the model (in setSelection() ).
	void itemsSelectedFromOutside(QVector<QGraphicsItem*> item);

	/// Is emitted when items are deselected within the model (in setSelection() ).
	void itemsDeselectedFromOutside(QVector<QGraphicsItem*> item);

	void updateScene();
};

#endif // MODEL_H
