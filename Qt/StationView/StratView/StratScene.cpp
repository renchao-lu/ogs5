/**
 * \file StratScene.cpp
 * 2010/03/16 - KR Initial implementation
 */

#include <limits>
#include <math.h>

#include <QGraphicsTextItem>

#include "StratScene.h"
#include "StratBar.h"
#include "QNonScalableGraphicsTextItem.h"
#include "DateTools.h"


StratScene::StratScene(GEOLIB::StationBorehole* station, QObject* parent) : QGraphicsScene(parent)
{
	QRectF textBounds;
	int stratBarOffset = 200;

	QFont font( "Arial" , 15, QFont::DemiBold, false);

	QNonScalableGraphicsTextItem* boreholeTag = addNonScalableText("Borehole", font);
	QNonScalableGraphicsTextItem* boreholeName = addNonScalableText("\"" + QString::fromStdString(station->getName()) + "\"", font);
	textBounds = boreholeTag->boundingRect();
	boreholeTag->setPos(50 + textBounds.width()/2, 60);
	textBounds = boreholeName->boundingRect();
	boreholeName->setPos(50 + textBounds.width()/2, 140);

	QNonScalableGraphicsTextItem* totalDepth = addNonScalableText("Depth: " + QString::number(station->getDepth()) + " m");
	textBounds = totalDepth->boundingRect();
	totalDepth->setPos(50 + textBounds.width()/2, 250);

	QNonScalableGraphicsTextItem* dateText = addNonScalableText("Date: " + QString::fromStdString(date2String(station->getDate())));
	textBounds = dateText->boundingRect();
	dateText->setPos(50 + textBounds.width()/2, 350);

	StratBar* stratBar = addStratBar(station);
	stratBar->setPos(stratBarOffset, MARGIN);
	QRectF stratBarBounds = stratBar->boundingRect();

	addDepthLabels(station->getProfile(), stratBarOffset + stratBarBounds.width());
	
	if (station->getSoilNames().size()>0) 
		addSoilNameLabels(station->getSoilNames(), station->getProfile(), stratBarOffset + (stratBarBounds.width()/2));

}

StratScene::~StratScene()
{
}

void StratScene::addDepthLabels(std::vector<GEOLIB::Point*> profile, double offset)
{
	QRectF textBounds;
	double vertPos = MARGIN;
	std::vector<QNonScalableGraphicsTextItem*> depthText;
	depthText.push_back(addNonScalableText(QString::number((*(profile[0]))[2])));
	textBounds = depthText[0]->boundingRect();
	depthText[0]->setPos(offset + textBounds.width()/2, vertPos);
	
	for (size_t i=1; i<profile.size(); i++)
	{
		depthText.push_back(addNonScalableText(QString::number((*(profile[i]))[2])));
		vertPos += log((*(profile[i-1]))[2]-(*(profile[i]))[2]+1)*100;
		textBounds = depthText[i]->boundingRect();
		depthText[i]->setPos(offset + textBounds.width()/2, vertPos);
	}
}

QNonScalableGraphicsTextItem* StratScene::addNonScalableText(const QString &text, const QFont &font)
{
    QNonScalableGraphicsTextItem *item = new QNonScalableGraphicsTextItem(text);
    item->setFont(font);
    addItem(item);
    return item;
}

void StratScene::addSoilNameLabels(std::vector<std::string> soilNames, std::vector<GEOLIB::Point*> profile, double offset)
{
	//QRectF textBounds;
	double vertPos = MARGIN, halfHeight = 0;
	std::vector<QNonScalableGraphicsTextItem*> soilText;
	soilText.push_back(addNonScalableText(QString::fromStdString(soilNames[0])));
	//textBounds = soilText[0]->boundingRect();
	soilText[0]->setPos(offset /* - textBounds.width() */, vertPos);
	
	for (size_t i=1; i<soilNames.size(); i++)
	{
		soilText.push_back(addNonScalableText(QString::fromStdString(soilNames[i])));
		halfHeight = log((*(profile[i-1]))[2]-(*(profile[i]))[2]+1)*100 / 2;
		//textBounds = soilText[i]->boundingRect();
		soilText[i]->setPos(offset /* - textBounds.width() */, vertPos + halfHeight);
		vertPos += ( 2 * halfHeight );
	}
}

StratBar* StratScene::addStratBar(GEOLIB::StationBorehole* station)
{
	StratBar* b = new StratBar(station);
	addItem(b);
	return b;
}
