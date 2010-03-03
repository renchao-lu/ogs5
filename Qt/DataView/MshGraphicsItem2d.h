/**
 * \file MshGraphicsItem2d.h
 * 29/10/2009 LB Initial implementation
 * 
 */


#ifndef MSHGRAPHICSITEM2D_H
#define MSHGRAPHICSITEM2D_H

// ** INCLUDES **
#include "GraphicsItem2d.h"
#include "msh_mesh.h"

#include <QObject>

/**
 * MshGraphicsItem2d
 */
class MshGraphicsItem2d : public GraphicsItem2d
{
	Q_OBJECT

public:
	MshGraphicsItem2d(Model* model, Mesh_Group::CFEMesh* mesh,
		EViewPlane viewplane = VIEWPLANE_XY, QGraphicsItem* parent = 0);
	
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	
	Mesh_Group::CFEMesh* mesh() const;

	void setEditable(bool enable);

public slots:
	void reloadElemItems();

	void updatePosition();

protected:


private:
	Mesh_Group::CFEMesh* _mesh;
	vector<GraphicsItem2d*> _elemItems;


};

#endif // MSHGRAPHICSITEM2D_H
