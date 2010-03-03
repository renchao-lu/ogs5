/**
 * \file Q2DDiagramScene.h
 * KR Initial implementation
 */

#ifndef Q2DDIAGRAMSCENE_H
#define Q2DDIAGRAMSCENE_H

#include <QGraphicsScene>
#include "QArrow.h"
#include "QDiagramList.h"
#include "QGraphicsGrid.h"
#include "QNonScalableGraphicsTextItem.h"

/**
 * A scene graph for a 2D Diagram including coordinate axes with 
 * labels and ticks for one or more plotted graphs.
 */
class Q2DDiagramScene : public QGraphicsScene
{
public:
	Q2DDiagramScene(QObject* parent = 0);
	Q2DDiagramScene(QDiagramList* list, QObject* parent = 0);
	~Q2DDiagramScene();

	QArrow* addArrow(float length, float angle, float headlength, float headwidth, QPen &pen);
	void addGraph(QDiagramList* list);
	QGraphicsGrid* addGrid(QRectF rect, int xTicks, int yTicks, QPen pen);

	static const int MARGIN=30;	/// The margin between the boundary of the scene and the bounding box of all items within the scene

private:
	void addCaption(QString name, QPen pen);
	QNonScalableGraphicsTextItem* addNonScalableText(const QString &text, const QFont &font = QFont());
	void adjustAxis(float &min, float &max, int &numberOfTicks);
	void adjustScaling();
	void clearGrid();
	void constructGrid();
	void drawGraph(QDiagramList* list);
	int getXAxisOffset();
	int getYAxisOffset();
	void initialize();
	void setDiagramBoundaries(QDiagramList* list);
	void setXAxis(QArrow* arrow);
	void setYAxis(QArrow* arrow);
	void update();

	QRectF _bounds;
	QRectF _unscaledBounds;
	QVector<QDiagramList*> _lists;
	QVector<QGraphicsItemGroup*> _graphCaptions;
	QVector<QGraphicsPathItem*> _graphs;
	QGraphicsGrid* _grid;
	float _scaleX;
	float _scaleY;
	QArrow* _xAxis;
	QArrow* _yAxis;
	QNonScalableGraphicsTextItem* _xLabel;
	QNonScalableGraphicsTextItem* _yLabel;
	QNonScalableGraphicsTextItem* _xUnit;
	QNonScalableGraphicsTextItem* _yUnit;
	QVector<QNonScalableGraphicsTextItem*> _xTicksText;
	QVector<QNonScalableGraphicsTextItem*> _yTicksText;
};

#endif //Q2DDIAGRAMSCENE_H
