/**
 * \file QDiagramList.cpp
 * KR Initial implementation
 */

#include <limits>
#include <QFile>
#include <QTextStream>
#include "QDiagramList.h"

/// Creates an empty list.
QDiagramList::QDiagramList()
{
	_xLabel = "";
	_yLabel = "";
	_xUnit = "";
	_yUnit = "";
}

QDiagramList::~QDiagramList()
{
}


/// Returns the minimum x-value of all the data points.
double QDiagramList::calcMinXValue()
{
	double min = numeric_limits<double>::max();
	for (int i=0; i<_xCoords.size(); i++)
	{
		if (_xCoords[i]<min) min=_xCoords[i];
	}
	return min;
}

/// Returns the maximum x-value of all the data points.
double QDiagramList::calcMaxXValue()
{
	double max = numeric_limits<double>::min();

	for (int i=0; i<_xCoords.size(); i++)
	{
		if (_xCoords[i]>max) max=_xCoords[i];
	}
	return max;
}

/// Returns the minimum y-value of all the data points.
double QDiagramList::calcMinYValue()
{
	double min = numeric_limits<double>::max();
	for (int i=0; i<_yCoords.size(); i++)
	{
		if (_yCoords[i]<min) min=_yCoords[i];
	}
	return min;
}

/// Returns the maximum y-value of all the data points.
double QDiagramList::calcMaxYValue()
{
	double max = numeric_limits<double>::min();

	for (int i=0; i<_yCoords.size(); i++)
	{
		if (_yCoords[i]>max) max=_yCoords[i];
	}
	return max;
}

/// Returns the colour of the graph.
QColor QDiagramList::getColor()
{
	return _colour;
}

QString QDiagramList::getName()
{
	return _name;
}

/**
 * Returns all the data points in form of a QPainterPath in scene coordinates.
 * The order of the points is the same as in the vector of coordinates.
 * \param path The path containing all data points.
 * \param scaleX Scaling factor in x-direction.
 * \param scaleY Scaling factor in y-direction.
 * \return true if everything is alright. false if the size of the coordinate arrays don't match.
 */
bool QDiagramList::getPath(QPainterPath &path, float scaleX, float scaleY)
{
	QPointF p;
	if (getPoint(p,0) && _xCoords.size()==_yCoords.size())
	{
		QPainterPath pp(QPointF(p.x()*scaleX, p.y()*scaleY));
		path = pp;
		
		for (int i=1; i<_xCoords.size(); i++)
		{
			getPoint(p,i);
			path.lineTo(QPointF(p.x()*scaleX, p.y()*scaleY));
		}
		return true;
	}
	else 
		return false;
}

/**
 * Returns the position of one point in the vector of coordinates.
 * \param p The point-object that will be returned.
 * \param i Number of the point to be returned.
 * \return true if everything is alright. false if the point does not exist.
 */
bool QDiagramList::getPoint(QPointF &p, int i)
{
	if (i<_xCoords.size())
	{
		p.setX(_xCoords[i]);
		p.setY(_yCoords[i]);
		return true;
	}
	else return false;
}

/// Returns the label associated with the x-axis
QString QDiagramList::getXLabel()
{
	return _xLabel;
}

/// Returns the label associated with the y-axis
QString QDiagramList::getYLabel()
{
	return _yLabel;
}

/// Returns the unit associated with the x-axis
QString QDiagramList::getXUnit()
{
	return _xUnit;
}

/// Returns the unit associated with the y-axis
QString QDiagramList::getYUnit()
{
	return _yUnit;
}

/// Returns the height of the bounding box of all data points within the list.
double QDiagramList::height()
{
	return (_maxY-_minY);
}

/// Returns the minimum x-value.
double QDiagramList::minXValue()
{
	return _minX;
}

/// Returns the maximum x-value.
double QDiagramList::maxXValue()
{
	return _maxX;
}

/// Returns the minimum y-value.
double QDiagramList::minYValue()
{
	return _minY;
}

/// Returns the maximum y-value.
double QDiagramList::maxYValue()
{
	return _maxY;
}

/*
 * Reads an external list into the coordinate arrays.
 * This method uses files containing the following format:
 *		xValue <tab> yValue
 * Both values may be int or double.
 */
/*
int QDiagramList::readList(char* path)
{
	int date;
	double xVal, yVal;
	QString line;
	QStringList fields;

	QFile file(path);
	QTextStream in( &file );

	if (!file.open(QIODevice::ReadOnly))
    {
		return 0;
	}

	while (!in.atEnd()) {
		line = in.readLine();
		fields = line.split('\t');
		if (fields.size() >= 2) {
			xVal = fields.takeFirst().toDouble();
			yVal = fields.takeFirst().toDouble();
			xCoords.push_back(xVal);
			yCoords.push_back(yVal);
		}
		else return 0;
	}
 
    file.close();
	update();
 
    return 1;
}*/

/**
 * Reads an ASCII file containing the coordinates in the following format:
 *		date (tab) value
 * where 'date' is given as 'dd.mm.yyyy'. 
 * (Changes to that format are easily implemented using QTimeDate)
 * \param path The path of the ASCII file.
 * \return Returns 1 if everything is alright. Returns 0 and displays an error message if there was an error.
 */
int QDiagramList::readList(QString path)
{
	int numberOfDays;
	double value;
	QDateTime date1, date2;
	QString line;
	QString stringDate;
	QStringList fields;

	QFile file(path);
	QTextStream in( &file );

	if (!file.open(QIODevice::ReadOnly))
    {
		qDebug("Could not open file...");
		return 0;
	}

	line = in.readLine();
	fields = line.split('\t');
	if (fields.size() >= 2) {
		stringDate = fields.takeFirst();
		value = fields.takeFirst().toDouble();
		date1 = QDateTime::fromString(stringDate, "dd.MM.yyyy");

		_xCoords.push_back(0);
		_yCoords.push_back(value);

		while (!in.atEnd()) {
			line = in.readLine();
			fields = line.split('\t');
			if (fields.size() >= 2) {
				stringDate = fields.takeFirst();
				value = fields.takeFirst().toDouble();
				date2 = QDateTime::fromString(stringDate, "dd.MM.yyyy");

				numberOfDays = date1.daysTo(date2);
				_xCoords.push_back(numberOfDays);
				_yCoords.push_back(value);
			}
			else return 0;
		}
	}
	else
	{
		qDebug("Unexpected file format...");
		return 0;
	}

	file.close();
	update();
 
    return 1;
}

/// Sets the colour of the graph.
void QDiagramList::setColor(QColor c)
{
	_colour = c;
}

/**
 * Sets the list of x/y-coordinates based on the two arrays that are given to this function.
 * Both arrays need to have the same size.
 * Note: This function converts the array QDateTime::x to an double-array 
 * containing the number of days from the first date (which is set as day 0)
 * \param x A list of dates.
 * \param y A list of values.
 * \return  Returns 1 if everything is alright. Returns 0 and an error message if the size of the coordinate arrays don't match.
 */
int QDiagramList::setList(vector<QDateTime> x, vector<double> y)
{
	int numberOfDays;
	QDateTime startDate;

	if (x.size() == y.size())
	{
		startDate = x[0];
		_xCoords.push_back(0);
		_yCoords.push_back(y[0]);

		for (size_t i=1; i<x.size(); i++)
		{
			numberOfDays = startDate.daysTo(x[i]);
			_xCoords.push_back(numberOfDays);
			_yCoords.push_back(y[i]);
		}
	}
	else
	{
		qDebug("QDiagramList::setList() - irregular vector size...");
		return 0;
	}

	update();
 
    return 1;
}

/**
 * Sets the list of x/y-coordinates based on the two arrays that are given to this function.
 * Both arrays need to have the same size.
 * \param x A list of values.
 * \param y A list of values.
 * \return  Returns 1 if everything is alright. Returns 0 and an error message if the size of the coordinate arrays don't match.
 */
int QDiagramList::setList(vector<double> x, vector<double> y)
{
	if (x.size() == y.size())
	{
		for (size_t i=0; i<x.size(); i++)
		{
			_xCoords.push_back(x[i]);
			_yCoords.push_back(y[i]);
		}
	}
	else
	{
		qDebug("QDiagramList::setList() - irregular vector size...");
		return 0;
	}
	update();
    return 1;
}


/// Sets the name of the graph to be displayed in the caption.
void QDiagramList::setName(QString n)
{
	_name = n;
}

/// Specifies the meaning of the x Axis.
void QDiagramList::setXLabel(QString label)
{
	_xLabel = label;
}

/// Specifies the meaning of the y Axis.
void QDiagramList::setYLabel(QString label)
{
	_yLabel = label;
}

/// Specifies the unit of the x Axis.
void QDiagramList::setXUnit(QString unit)
{
	_xUnit = unit;
}

/// Specifies the unit of the y Axis.
void QDiagramList::setYUnit(QString unit)
{
	_yUnit = unit;
}

/// Returns the number of data points.
int QDiagramList::size()
{
	if (!(_xCoords.isEmpty() || _yCoords.isEmpty()))
	{
		if (_xCoords.size() == _yCoords.size()) 
			return _xCoords.size();
		return -1;
	}
	else return -1;
}

/// Updates the bounds of the data points contained in the list.
void QDiagramList::update()
{
	_minX = calcMinXValue();
	_maxX = calcMaxXValue();
	_minY = calcMinYValue();
	_maxY = calcMaxYValue();
}

/// Returns the width of the bounding box of all data points within the list.
double QDiagramList::width()
{
	return _maxX-_minX;
}
