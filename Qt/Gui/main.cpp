#include <QtGui/QApplication>
#include "mainwindow.h"
//#include "geo_lib.h"
//#include "msh_lib.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	setlocale(LC_NUMERIC,"C");
	MainWindow w;
	w.show();
	return a.exec();
}
