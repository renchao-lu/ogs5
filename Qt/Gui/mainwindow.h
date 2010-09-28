/**
* \file mainwindow.h
* 4/11/2009 LB Initial implementation
*
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "DatabaseConnection.h"
#include <QStringList>

class StationTreeModel;
class GEOModels;
class MshModel;
class VtkVisPipeline;

class MainWindow : public QMainWindow, public Ui_MainWindowClass
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void ShowWindow();
	void HideWindow();

protected:
	void closeEvent( QCloseEvent* event );

protected slots:
	void showPntDockWidget( bool show );
	void showLineDockWidget( bool show );
	void showSurfaceDockWidget( bool show );
	void showStationDockWidget( bool show );
	void showMshDockWidget( bool show );
	void showVisDockWidget( bool show );

private slots:
    void open();
	void save();
	void exportBoreholesToGMS(std::string listName, std::string fileName);
    void importGMS();
	void importGoCad();
	void importRaster();
	void importShape();
	void importPetrel();
	void importNetcdf();     //YW  07.2010
	void openDatabase();
	void openDatabaseConnection();
	void openRecentFile();
	void about();
	void showDiagramPrefsDialog(QModelIndex &index);
	void showPropertiesDialog(std::string name);
	void showVisalizationPrefsDialog();
	void showTrackingSettingsDialog();
	void updateDataViews();
	void showAddPipelineFilterItemDialog(QModelIndex parentIndex);

	void on_actionExportVTK_triggered(bool checked = false);
	void on_actionExportVRML2_triggered(bool checked = false);
	void on_actionExportObj_triggered(bool checked = false);
	void on_actionExportOpenSG_triggered(bool checked = false);

private:
	QMenu* createImportFilesMenu();
    void loadFile(const QString &fileName);
    void loadPetrelFiles(const QStringList &sfc_file_names, const QStringList &well_path_file_names);

	void readSettings();
	void writeSettings();

    QString curFile;

	DatabaseConnection* _db;
	GEOModels* _geoModels;
	MshModel* _meshModels;
	VtkVisPipeline* _vtkVisPipeline;

signals:
	void fileUsed( QString filename );
};

class StartQt4
{
public:
	StartQt4()
	{
		int i = 0;
		QApplication* qapp = new QApplication(i, NULL);
		qapp->exec();
	}
};

#endif // MAINWINDOW_H
