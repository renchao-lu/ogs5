/**
 * \file QDiagramList.h
 * KR Initial implementation
 */

#ifndef QDIAGRAMLIST_H
#define QDIAGRAMLIST_H

#include <vector>
#include <QPoint>
#include <QPainterPath>
#include <QColor>
#include <QDateTime>


using namespace std;

/**
 * A List of data points and all the necessary meta-information to draw a graph.
 */
class QDiagramList
{
public:
	QDiagramList();
	~QDiagramList();

	QColor getColor();
	double height();
	double minXValue();
	double maxXValue();
	double minYValue();
	double maxYValue();
	QString getName();
	bool getPath(QPainterPath &path, float scaleX, float scaleY);
	bool getPoint(QPointF &p, int i);
	QString getXLabel();
	QString getYLabel();
	QString getXUnit();
	QString getYUnit();
	int readList(QString path);
	void setColor(QColor c);
	void setName(QString name);
	void setXLabel(QString name);
	void setYLabel(QString name);
	int setList(vector<double> x, vector<double> y);
	int setList(vector<QDateTime> x, vector<double> y);
	void setXUnit(QString unit);
	void setYUnit(QString unit);
	int size();
	double width();

private:
	double calcMinXValue();
	double calcMaxXValue();
	double calcMinYValue();
	double calcMaxYValue();	
	int readLine(ifstream inFile, QDateTime &cDate, double &cValue);
	void update();

	double _maxX;
	double _maxY;
	double _minX;
	double _minY;
	QVector<double> _xCoords;
	QVector<double> _yCoords;
	QString _name;
	QString _xLabel;
	QString _yLabel;
	QString _xUnit;
	QString _yUnit;
	QColor _colour;

};

#endif //QDIAGRAMLIST_H
