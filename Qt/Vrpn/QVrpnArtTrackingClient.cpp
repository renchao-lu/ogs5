/**
 * \file QVrpnArtTrackingClient.cpp
 * 03/09/2010 LB Initial implementation
 * 
 * Implementation of QVrpnArtTrackingClient class
 */

// ** INCLUDES **
#include "QVrpnArtTrackingClient.h"

#include <iostream>


QVrpnArtTrackingClient* QVrpnArtTrackingClient::_singleton = 0;

QVrpnArtTrackingClient::QVrpnArtTrackingClient(QObject* parent /*= NULL*/)
: QObject(parent), VrpnArtTrackingClient()
{
	//SpaceNavigatorClient::Instance();
	_timer = new QTimer(this);
	connect( _timer, SIGNAL(timeout()), this, SLOT(MainLoop()) );
}

QVrpnArtTrackingClient::~QVrpnArtTrackingClient()
{
}

QVrpnArtTrackingClient* QVrpnArtTrackingClient::Instance(QObject* parent /*= NULL*/)
{
	if(_singleton == 0)
		_singleton = new QVrpnArtTrackingClient(parent);
	return _singleton;
}

void QVrpnArtTrackingClient::StartTracking(const char *deviceName,
	int updateInterval /*= 100*/)
{
	VrpnArtTrackingClient::StartTracking(deviceName);
	std::cout << "Tracking started." << std::endl;
	if (updateInterval > 0)
		_timer->start(updateInterval);
}

void QVrpnArtTrackingClient::MainLoop()
{
	VrpnArtTrackingClient::MainLoop();
	
	double x, y, z;
	VrpnArtTrackingClient::GetBodyTranslation(x, y, z);
	std::cout << "Body: " << x << " " << y << " " << z << std::endl;
	//std::cout << "Body: " << m_dBodyTranslation[0] << " " << m_dBodyTranslation[1] << " " << m_dBodyTranslation[2] << std::endl;
	emit positionUpdated(x, z, y);
	
	/*
	if (_unconsumedData)
	{
		_unconsumedData = false;
		double x, y, z, rx, ry, rz;
		getTranslation(x, y, z);
		getRotation(rx, ry, rz);
		//emit updated(x, y, z, rx, ry, rz);
		emit translated(x, y, z);
	}
	*/
}
