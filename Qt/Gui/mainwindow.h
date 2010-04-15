/**
* \file mainwindow.h
* 4/11/2009 LB Initial implementation
*
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "DatabaseConnection.h"
//#include <QtGui/QMainWindow>
#include <QStringList>

class StationTreeModel;
class GEOModels;
class VtkVisPipeline;

class MainWindow : public QMainWindow, public Ui_MainWindowClass
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
	void closeEvent( QCloseEvent* event );

protected slots:
	void showPntDockWidget( bool show );
	void showLineDockWidget( bool show );
	void showStationDockWidget( bool show );
	void showMshDockWidget( bool show );

private slots:
    void open();
	void save();
	void exportBoreholesToGMS(std::string listName, std::string fileName);
    void importGoCad();
	void importRaster();
	void importShape();
	void importPetrel();
	void openDatabase();
	void openDatabaseConnection();
	void openRecentFile();
	void about();
	void setConnectsSceneToTabWidgets(int type);
	void setConnectsStationsToDiagramView();
	void showDiagramPrefs(QModelIndex &idx);
	void showPropertiesDialog(std::string name);
	void updateGraphicsScene();
	void showAddPipelineFilterItemDialog(QModelIndex parentIndex);

private:
	QMenu* createImportFilesMenu();
    void loadFile(const QString &fileName);
    void loadPetrelFiles(const QStringList &sfc_file_names, const QStringList &well_path_file_names);

	void readSettings();
	void writeSettings();

    QString curFile;

	DatabaseConnection* _db;
	GEOModels* _geoModels;
	VtkVisPipeline* _vtkVisPipeline;

signals:
	void fileUsed( QString filename );
};

#endif // MAINWINDOW_H
