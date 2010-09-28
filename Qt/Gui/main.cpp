#include <QtGui/QApplication>
#include "mainwindow.h"
#include <OpenSG/OSGBaseFunctions.h>

int main(int argc, char *argv[])
{
	// OpenSG
	OSG::osgInit(argc,argv);

	QApplication a(argc, argv);
	setlocale(LC_NUMERIC,"C");
	MainWindow w;
	w.show();
	return a.exec();
}
