/**
 * \file ModelItem.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef MODELITEM_H
#define MODELITEM_H

// ** INCLUDES **
#include <QObject>
#include <vector>

class GraphicsItem2d;
class Model;

using namespace std;

/**
 * The ModelItem is used to store 2d and 3d graphic items in models.
 * It can also have a pointer to a dependent model
 */
class ModelItem : public QObject
{
	Q_OBJECT

public:
	/**
	 * \param item2d The 2d graphics items
	 * \param model The dependent model
	 */
	ModelItem(GraphicsItem2d* const item2d, QObject* parent = 0);

	/// Deletes 2d and 3d items
	~ModelItem();
	
	/// Returns the 2d graphics item
	GraphicsItem2d* item2d();

	/// Returns the number of children (used for tree models)
	virtual int childCount() const;

	/// Sets the number of children (used for tree models)
	void setNumberOfChildren(int numberOfChildren);

	/// Returns the dependent models
	vector<Model*> models() const;

	/// Adds a dependent model
	void addModel(Model* model);

private:
	GraphicsItem2d* const _item2d;
	vector<Model*> _models;

	int _numberOfChildren;
};

#endif // MODELITEM_H
