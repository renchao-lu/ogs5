/**
 * \file GraphicsView2d.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef GRAPHICSVIEW2D_H
#define GRAPHICSVIEW2D_H

// ** INCLUDES **
#include <QGraphicsView>

class GraphicsScene;

/**
 * The GraphicsView2d provides an OpenGL accelerated 2d view with 
 * background pattern. Mouse navigation includes mouse wheel zooming and 
 * panning through holding down the shift key. The GraphicsScene is
 * used and the y-coordinate is flipped and so points upwards.
 */
class GraphicsView2d : public QGraphicsView
{
	Q_OBJECT

public:
	GraphicsView2d(QWidget *parent = 0);

	/// Returns the graphics scene
	GraphicsScene* scene() const;

	/// Sets the graphics scene.
	void setScene(GraphicsScene* scene);

public slots:
	/// This should be called whenever the scene changes and resizes some items.
	void updateView();
	
protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);
	void resizeEvent(QResizeEvent *event);
	//void drawBackground(QPainter *painter, const QRectF &rect);

	void scaleView(qreal scaleFactor);
	

private:
	GraphicsScene* _graphicsScene;

};

#endif // GRAPHICSVIEW2D_H
