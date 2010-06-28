/**
* \file mainwindow.h
* 4/11/2009 LB Initial implementation
*
*/

#include "mainwindow.h"
#include "msh_lib.h"
#include "PntsModel.h"
#include "LinesModel.h"
#include "StationTreeModel.h"
#include "MshModel.h"
#include "OGSError.h"
#include "DBConnectionDialog.h"
#include "DiagramPrefsDialog.h"
#include "SHPImportDialog.h"
#include "OGSRaster.h"
#include "GEOModels.h"
#include "ListPropertiesDialog.h"
#include "Configure.h"
#include "VtkVisPipeline.h"
#include "VtkAddFilterDialog.h"
#include "RecentFiles.h"
#include "VisPrefsDialog.h"

// GEOLIB includes
#include "Point.h"
#include "Polyline.h"
#include "Station.h"

// FileIO includes
#include "OGSIOVer4.h"
#include "StationIO.h"
#include "PetrelInterface.h"
#include "GocadInterface.h"
#include "XMLInterface.h"
#include "GMSHInterface.h"
#include "GMSInterface.h"

// Qt Includes
#include <QFileDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QSettings>
#include <QComboBox>
#include <QImage>
#include <QPixmap>

//vtk testing
#include <vtkRenderWindow.h>
#include "VtkBGImageSource.h"
#include <vtkVRMLExporter.h>
#include <vtkOBJExporter.h>

#include <clocale>

// for sizeof operator
//#include "rf_st_new.h"
//#include "rf_bc_new.h"

/// FEM. 11.03.2010. WW
#include "problem.h"
Problem *aproblem = NULL;

using namespace FileIO;


MainWindow::MainWindow(QWidget *parent /* = 0*/)
: QMainWindow(parent), _db (NULL)
{
    setupUi(this);

	// Setup connection GEOObjects to GUI through GEOModels and tab widgets
	_geoModels = new GEOModels();

	// station model
	stationTabWidget->treeView->setModel(_geoModels->getStationModel());
	connect(stationTabWidget->treeView, SIGNAL(stationListExportRequested(std::string, std::string)),
		this, SLOT(exportBoreholesToGMS(std::string, std::string)));	// export Stationlist to GMS
	connect(stationTabWidget->treeView, SIGNAL(stationListRemoved(std::string)),
		_geoModels, SLOT(removeStationVec(std::string)));	// update model when stations are removed
	connect(_geoModels, SIGNAL(stationVectorRemoved(StationTreeModel*, std::string)),
		this, SLOT(updateDataViews()));						// update data view when stations are removed
	connect(stationTabWidget->treeView, SIGNAL(diagramRequested(QModelIndex&)),
		this, SLOT(showDiagramPrefsDialog(QModelIndex&)));		// connect treeview to diagramview


	// point models
	connect (_geoModels, SIGNAL(pointModelAdded(Model*)),
		pntTabWidget->dataViewWidget, SLOT(addModel(Model*)));
	connect(pntTabWidget->dataViewWidget, SIGNAL(requestModelClear(std::string)),
		_geoModels, SLOT(removePointVec(const std::string)));
	connect (_geoModels, SIGNAL(pointModelRemoved(Model*)),
		pntTabWidget->dataViewWidget, SLOT(removeModel(Model*)));
	connect(_geoModels, SIGNAL(pointModelRemoved(Model*)),
		this, SLOT(updateDataViews()));

	// line models
	connect (_geoModels, SIGNAL(polylineModelAdded(Model*)),
		lineTabWidget->dataViewWidget, SLOT(addModel(Model*)));
	connect(lineTabWidget->dataViewWidget, SIGNAL(requestModelClear(std::string)),
		_geoModels, SLOT(removePolylineVec(const std::string)));
	connect (_geoModels, SIGNAL(polylineModelRemoved(Model*)),
		lineTabWidget->dataViewWidget, SLOT(removeModel(Model*)));
	connect(_geoModels, SIGNAL(polylineModelRemoved(Model*)),
		this, SLOT(updateDataViews()));

	// surface models
	connect (_geoModels, SIGNAL(surfaceModelAdded(Model*)),
		surfaceTabWidget->dataViewWidget, SLOT(addModel(Model*)));
	connect(surfaceTabWidget->dataViewWidget, SIGNAL(requestModelClear(std::string)),
		_geoModels, SLOT(removeSurfaceVec(const std::string)));
	connect (_geoModels, SIGNAL(surfaceModelRemoved(Model*)),
		surfaceTabWidget->dataViewWidget, SLOT(removeModel(Model*)));
	connect(_geoModels, SIGNAL(surfaceModelRemoved(Model*)),
		this, SLOT(updateDataViews()));


	// Setup connections for mesh models to GUI
	_meshModels = new MshModel();
	mshTabWidget->treeView->setModel(_meshModels);
	connect(mshTabWidget, SIGNAL(requestMeshRemoval(const QModelIndex&)),
		_meshModels, SLOT(removeMesh(const QModelIndex&)));

	// vtk visualization pipeline
	_vtkVisPipeline = new VtkVisPipeline(visualizationWidget->renderer());
	connect(_geoModels, SIGNAL(pointModelAdded(Model*)),
		_vtkVisPipeline, SLOT(addPipelineItem(Model*)));
	connect(_geoModels, SIGNAL(pointModelRemoved(Model*)),
		_vtkVisPipeline, SLOT(removeSourceItem(Model*)));
	connect(_geoModels, SIGNAL(polylineModelAdded(Model*)),
		_vtkVisPipeline, SLOT(addPipelineItem(Model*)));
	connect(_geoModels, SIGNAL(polylineModelRemoved(Model*)),
		_vtkVisPipeline, SLOT(removeSourceItem(Model*)));
	connect(_geoModels, SIGNAL(surfaceModelAdded(Model*)),
		_vtkVisPipeline, SLOT(addPipelineItem(Model*)));
	connect(_geoModels, SIGNAL(surfaceModelRemoved(Model*)),
		_vtkVisPipeline, SLOT(removeSourceItem(Model*)));
	connect(_geoModels, SIGNAL(stationVectorAdded(StationTreeModel*, std::string)),
		_vtkVisPipeline, SLOT(addPipelineItem(StationTreeModel*, std::string)));
	connect(_geoModels, SIGNAL(stationVectorRemoved(StationTreeModel*, std::string)),
		_vtkVisPipeline, SLOT(removeSourceItem(StationTreeModel*, std::string)));
	connect(_meshModels, SIGNAL(meshAdded(MshModel*, QModelIndex)),
		_vtkVisPipeline, SLOT(addPipelineItem(MshModel*,QModelIndex)));
	connect(_meshModels, SIGNAL(meshRemoved(MshModel*, QModelIndex)),
		_vtkVisPipeline, SLOT(removeSourceItem(MshModel*, QModelIndex)));

	connect(_vtkVisPipeline, SIGNAL(vtkVisPipelineChanged()),
		visualizationWidget->vtkWidget, SLOT(update()));
	connect(_vtkVisPipeline, SIGNAL(vtkVisPipelineChanged()),
		vtkVisTabWidget->vtkVisPipelineView, SLOT(expandAll()));

	vtkVisTabWidget->vtkVisPipelineView->setModel(_vtkVisPipeline);
	connect(vtkVisTabWidget->vtkVisPipelineView, SIGNAL(requestRemovePipelineItem(QModelIndex)),
		_vtkVisPipeline, SLOT(removePipelineItem(QModelIndex)));
	connect(vtkVisTabWidget->vtkVisPipelineView, SIGNAL(requestAddPipelineFilterItem(QModelIndex)),
		this, SLOT(showAddPipelineFilterItemDialog(QModelIndex)));
	connect(vtkVisTabWidget, SIGNAL(requestViewUpdate()),
		visualizationWidget, SLOT(updateView()));


	// Stack the data dock widgets together
	tabifyDockWidget(pntDock, lineDock);
	tabifyDockWidget(lineDock, stationDock);
	tabifyDockWidget(surfaceDock, stationDock);
	tabifyDockWidget(stationDock, mshDock);

	// Restore window geometry
	readSettings();

	// Setup import files menu
	menu_File->insertMenu( action_Exit, createImportFilesMenu() );

	// Setup recent files menu
	RecentFiles* recentFiles = new RecentFiles(this, SLOT(openRecentFile()), "recentFileList", "OpenGeoSys-5");
	connect(this, SIGNAL(fileUsed(QString)), recentFiles, SLOT(setCurrentFile(QString)));
	menu_File->insertMenu( action_Exit, recentFiles->menu() );

	// Setup Windows menu
	QAction* showPntDockAction = pntDock->toggleViewAction();
	showPntDockAction->setStatusTip(tr("Shows / hides the points view"));
	connect(showPntDockAction, SIGNAL(triggered(bool)), this, SLOT(showPntDockWidget(bool)));
	menuWindows->addAction(showPntDockAction);

	QAction* showLineDockAction = lineDock->toggleViewAction();
	showLineDockAction->setStatusTip(tr("Shows / hides the lines view"));
	connect(showLineDockAction, SIGNAL(triggered(bool)), this, SLOT(showLineDockWidget(bool)));
	menuWindows->addAction(showLineDockAction);

	QAction* showStationDockAction = stationDock->toggleViewAction();
	showStationDockAction->setStatusTip(tr("Shows / hides the lines view"));
	connect(showStationDockAction, SIGNAL(triggered(bool)), this, SLOT(showStationDockWidget(bool)));
	menuWindows->addAction(showStationDockAction);

	QAction* showSurfaceDockAction = surfaceDock->toggleViewAction();
	showSurfaceDockAction->setStatusTip(tr("Shows / hides the surface view"));
	connect(showSurfaceDockAction, SIGNAL(triggered(bool)), this, SLOT(showSurfaceDockWidget(bool)));
	menuWindows->addAction(showSurfaceDockAction);

	QAction* showMshDockAction = mshDock->toggleViewAction();
	showMshDockAction->setStatusTip(tr("Shows / hides the lines view"));
	connect(showMshDockAction, SIGNAL(triggered(bool)), this, SLOT(showMshDockWidget(bool)));
	menuWindows->addAction(showMshDockAction);

	QAction* showVisDockAction = vtkVisDock->toggleViewAction();
	showVisDockAction->setStatusTip(tr("Shows / hides the visualization pipeline view"));
	connect(showVisDockAction, SIGNAL(triggered(bool)), this, SLOT(showVisDockWidget(bool)));
	menuWindows->addAction(showVisDockAction);

	// connects for point model
	//connect(pntTabWidget->pointsTableView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
	//	pntsModel, SLOT(setSelectionFromOutside(const QItemSelection&, const QItemSelection&)));
	//connect(pntTabWidget->clearAllPushButton, SIGNAL(clicked()), pntsModel, SLOT(clearData()));
	//connect(pntTabWidget->clearSelectedPushButton, SIGNAL(clicked()), pntsModel, SLOT(clearSelectedData()));
	//connect(pntTabWidget->clearAllPushButton, SIGNAL(clicked()), linesModel, SLOT(clearData()));

	// connects for station model
	connect(stationTabWidget->treeView, SIGNAL(propertiesDialogRequested(std::string)), this, SLOT(showPropertiesDialog(std::string)));


//	std::cout << "size of Point: " << sizeof (GEOLIB::Point) << std::endl;
//	std::cout << "size of CGLPoint: " << sizeof (CGLPoint) << std::endl;
//
//	std::cout << "size of Polyline: " << sizeof (GEOLIB::Polyline) << std::endl;
//	std::cout << "size of CGLPolyline: " << sizeof (CGLPolyline) << std::endl;
//
//	std::cout << "size of GEOLIB::Surface: " << sizeof (GEOLIB::Surface) << std::endl;
//	std::cout << "size of Surface: " << sizeof (Surface) << std::endl;
//
//	std::cout << "size of CCore: " << sizeof (Mesh_Group::CCore) << std::endl;
//	std::cout << "size of CNode: " << sizeof (Mesh_Group::CNode) << std::endl;
//	std::cout << "size of CElement: " << sizeof (Mesh_Group::CNode) << std::endl;
//	std::cout << "size of CEdge: " << sizeof (Mesh_Group::CEdge) << std::endl;
//	std::cout << "size of CFEMesh: " << sizeof (Mesh_Group::CFEMesh) << std::endl;
//	std::cout << "size of Matrix: " << sizeof (Math_Group::Matrix) << std::endl;
//
//	std::cout << "size of vec<size_t>: " << sizeof (Math_Group::vec<size_t>) << std::endl;
//	std::cout << "size of std::vector: " << sizeof (std::vector<size_t>) << std::endl;

//	std::cout << "size of CSourceTerm: " << sizeof (CSourceTerm) << std::endl;
//	std::cout << "size of CBoundaryCondition: " << sizeof (CBoundaryCondition) << std::endl;
}

MainWindow::~MainWindow()
{
	delete _db;
	delete _vtkVisPipeline;
	delete _meshModels;
	delete _geoModels;
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	writeSettings();
	QWidget::closeEvent(event);
}

void MainWindow::showPntDockWidget( bool show )
{
	if (show)
		pntDock->show();
	else
		pntDock->hide();
}
void MainWindow::showLineDockWidget( bool show )
{
	if (show)
		lineDock->show();
	else
		lineDock->hide();
}

void MainWindow::showStationDockWidget( bool show )
{
	if (show)
		stationDock->show();
	else
		stationDock->hide();
}

void MainWindow::showSurfaceDockWidget( bool show )
{
	if (show)
		surfaceDock->show();
	else
		surfaceDock->hide();
}

void MainWindow::showMshDockWidget( bool show )
{
	if (show)
		mshDock->show();
	else
		mshDock->hide();
}

void MainWindow::open()
{
	QSettings settings("UFZ", "OpenGeoSys-5");
    QString fileName = QFileDialog::getOpenFileName(this,
		"Select data file to open", settings.value("lastOpenedFileDirectory").toString(),
		"Geosys files (*.gli *.gml *.msh *.stn);;GLI files (*.gli);;MSH files (*.msh);;STN files (*.stn);;All files (* *.*)");
     if (!fileName.isEmpty())
	 {
		QDir dir = QDir(fileName);
		settings.setValue("lastOpenedFileDirectory", dir.absolutePath());
		loadFile(fileName);
		std::cout << dir.absolutePath().toStdString() << std::endl;
     }
}


void MainWindow::openDatabase()
{
	if (_db==NULL)
	{
		_db = new DatabaseConnection(_geoModels);
		_db->dbConnect();
	}

	if (_db!=NULL && _db->isConnected())
	{
		_db->getListSelection();
		updateDataViews();
	}
}

void MainWindow::openDatabaseConnection()
{
	if (_db==NULL)
		_db = new DatabaseConnection(_geoModels);
	DBConnectionDialog* dbConn = new DBConnectionDialog();
    connect(dbConn, SIGNAL(connectionRequested(QString, QString, QString, QString, QString)), _db, SLOT(setConnection(QString, QString, QString, QString, QString)));
    dbConn->show();
}


void MainWindow::openRecentFile()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
		loadFile(action->data().toString());
}

void MainWindow::save()
{
	QSettings settings("UFZ", "OpenGeoSys-5");
	QStringList files = settings.value("recentFileList").toStringList();
	QString dir_str;
	if (files.size() != 0) dir_str = QFileInfo(files[0]).absolutePath();
	else dir_str = QDir::homePath();

    QString fileName = QFileDialog::getSaveFileName(this, "Save data as", dir_str,"Geosys XML files (*.gml);;All files (* *.*)");

	QString gliName = pntTabWidget->dataViewWidget->modelSelectComboBox->currentText();

	if (!fileName.isEmpty())
	{
		QFile file(fileName);
		file.open( QIODevice::WriteOnly );

		std::string schemaName(SOURCEPATH);
		schemaName.append("/OpenGeoSysGLI.xsd");
		XMLInterface xml(_geoModels, schemaName);
		xml.writeGLIFile(file, gliName);
		file.close();

		xml.insertStyleFileDefinition(fileName);

		std::cout << "writing " << fileName.toStdString () << std::endl;
		GMSHInterface gmsh_io;
		gmsh_io.writeGMSHInputFile(fileName.toStdString(), gliName.toStdString(), *_geoModels);
	}
	else OGSError::box("No file name entered.");
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Application"),
                          tr("Cannot read file %1:\n%2.")
                          .arg(fileName)
                          .arg(file.errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFileInfo fi(fileName);
    string base = fi.absoluteDir().absoluteFilePath(fi.completeBaseName()).toStdString();
    if (fi.suffix().toLower() == "gli") {
#ifndef NDEBUG
    	 QTime myTimer0;
    	 myTimer0.start();
#endif
//    	FileIO::readGLIFileV4 (fileName.toStdString(), _geoModels);
    	readGLIFileV4 (fileName.toStdString(), _geoModels);
#ifndef NDEBUG
    	std::cout << myTimer0.elapsed() << " ms" << std::endl;
#endif
//
//#ifndef NDEBUG
//    	QTime myTimer;
//    	myTimer.start();
//    	std::cout << "GEOLIB_Read_GeoLib ... " << std::flush;
//#endif
//    	GEOLIB_Read_GeoLib(base); //fileName.toStdString());
//        cout << "Nr. Points: " << gli_points_vector.size() << endl;
//		cout << "Nr. Lines: " << polyline_vector.size() << endl;
//		cout << "Nr. Surfaces: " << surface_vector.size() << endl;
//#ifndef NDEBUG
//    	 std::cout << myTimer.elapsed() << " ms" << std::endl;
//#endif
// 		GEOCalcPointMinMaxCoordinates();
    }
	else if (fi.suffix().toLower() == "gml")
	{
#ifndef NDEBUG
    	 QTime myTimer0;
    	 myTimer0.start();
#endif
		std::string schemaName(SOURCEPATH);
		schemaName.append("/OpenGeoSysGLI.xsd");
//		XMLInterface xml(_geoModels, schemaName);
//		xml.readGLIFile(fileName);
#ifndef NDEBUG
    	std::cout << myTimer0.elapsed() << " ms" << std::endl;
#endif
	}
	else if (fi.suffix().toLower() == "stn")
	{
		GEOLIB::Station::StationType type = GEOLIB::Station::BOREHOLE;
		vector<GEOLIB::Point*> *stations = new vector<GEOLIB::Point*>();
		string name;

		if (int returnValue = StationIO::readStationFile(fileName.toStdString(), name, stations, type))
		{
			if (returnValue<0) cout << "main(): An error occured while reading the file.\n";

			if (type == GEOLIB::Station::BOREHOLE)
			{
				QString filename = QFileDialog::getOpenFileName(this, tr("Open stratigraphy file"), "", tr("Station Files (*.stn)"));

				/* read stratigraphy for all boreholes at once */
				vector<GEOLIB::Point*> *boreholes = new vector<GEOLIB::Point*>();
				size_t vectorSize = stations->size();
				for (size_t i=0; i<vectorSize; i++) boreholes->push_back(static_cast<GEOLIB::StationBorehole*>(stations->at(i)));
				GEOLIB::StationBorehole::addStratigraphies(filename.toStdString(), boreholes);
				for (size_t i=0; i<vectorSize; i++) (*stations)[i] = (*boreholes)[i];
				delete boreholes;
				/* read stratigraphy for each borehole seperately *
				for (int i=0; i<static_cast<int>(stations.size());i++)
				{
				StationBorehole* borehole = static_cast<StationBorehole*>(stations.at(i));
				StationBorehole::addStratigraphy(filename.toStdString(), borehole);
				if (borehole->find("q")) stations.at(i)->setColor(255,0,0);
				}
				*/
			}

			_geoModels->addStationVec(stations, name, GEOLIB::getRandomColor());
		}
	}
    else if (fi.suffix().toLower() == "msh")
	{
#ifndef NDEBUG

    	 QTime myTimer;
    	 myTimer.start();
    	 std::cout << "FEMRead ... " << std::flush;
#endif

		 FEMRead(base);
#ifndef NDEBUG
    	 std::cout << myTimer.elapsed() << " ms" << std::endl;
#endif

        CompleteMesh();
        if (fem_msh_vector.size() == 0)
		{
            cout << "Failed to load a mesh file: base path = " << base << endl;
        	return;
        }
        cout << "Nr. Nodes: " << ::fem_msh_vector[0]->nod_vector.size() << endl;

#ifndef NDEBUG
    	 myTimer.start();
    	 std::cout << "GridAdapter Read ... " << std::flush;
#endif

		 //GridAdapter grid(fileName.toStdString());				// load mesh-files directly into GridAdapter
		 GridAdapter* grid = new GridAdapter(fem_msh_vector[0]);	// convert CFEMeshes to GridAdapter
#ifndef NDEBUG
    	 std::cout << myTimer.elapsed() << " ms" << std::endl;
#endif
		 std::string name = (fi.baseName()).toStdString();
		 _meshModels->addMesh(grid, name);
	}
	else if (fi.suffix().toLower() == "txt")
	{
		vector<GEOLIB::Point*> *boreholes = new vector<GEOLIB::Point*>();
		string name = fi.baseName().toStdString();

		if (GMSInterface::readBoreholesFromGMS(boreholes, fileName.toStdString()))
			_geoModels->addStationVec(boreholes, name, GEOLIB::getRandomColor());
	}
	else if (fi.suffix().toLower() == "ts") {
#ifndef NDEBUG
    	 QTime myTimer;
    	 myTimer.start();
    	 std::cout << "GoCad Read ... " << std::flush;
#endif
    	 FileIO::GocadInterface (fileName.toStdString(), _geoModels);
#ifndef NDEBUG
    	 std::cout << myTimer.elapsed() << " ms" << std::endl;
#endif
	}

	updateDataViews();

	emit fileUsed(fileName);
}

void MainWindow::loadPetrelFiles(const QStringList &sfc_file_names, const QStringList &well_path_file_names)
{
	QStringList::const_iterator it = sfc_file_names.begin();
	std::list<std::string> sfc_files;
	while(it != sfc_file_names.end()) {
		sfc_files.push_back ((*it).toStdString());
	    ++it;
	}

	it = well_path_file_names.begin();
	std::list<std::string> well_path_files;
	while(it != well_path_file_names.end()) {
		well_path_files.push_back ((*it).toStdString());
	    ++it;
	}

	std::string unique_str (*(sfc_files.begin()));

	PetrelInterface (sfc_files, well_path_files, unique_str, _geoModels);
}

void MainWindow::updateDataViews()
{
	visualizationWidget->showAll();
	pntTabWidget->		dataViewWidget->dataView->updateView();
	lineTabWidget->		dataViewWidget->dataView->updateView();
	surfaceTabWidget->	dataViewWidget->dataView->updateView();
	stationTabWidget->	treeView->updateView();
	mshTabWidget->		treeView->updateView();

    QApplication::restoreOverrideCursor();
}


void MainWindow::readSettings()
{
	QSettings settings("UFZ", "OpenGeoSys-5");

	restoreGeometry(settings.value("windowGeometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::writeSettings()
{
	QSettings settings("UFZ", "OpenGeoSys-5");

	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("windowState", saveState());
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About OpenGeoSys-5"), tr("Built on %1").
		arg(QDate::currentDate().toString()));
}

QMenu* MainWindow::createImportFilesMenu()
{
	QMenu* importFiles = new QMenu("Import Files");
	QAction* gmsFiles = importFiles->addAction("GMS Files...");
	connect(gmsFiles, SIGNAL(triggered()), this, SLOT(importGMS()));
	QAction* gocadFiles = importFiles->addAction("Gocad Files...");
	connect(gocadFiles, SIGNAL(triggered()), this, SLOT(importGoCad()));
	QAction* petrelFiles = importFiles->addAction("Petrel Files...");
	connect(petrelFiles, SIGNAL(triggered()), this, SLOT(importPetrel()));
	QAction* rasterFiles = importFiles->addAction("Raster Files...");
	connect(rasterFiles, SIGNAL(triggered()), this, SLOT(importRaster()));
	QAction* shapeFiles = importFiles->addAction("Shape Files...");
	connect(shapeFiles, SIGNAL(triggered()), this, SLOT(importShape()));

	return importFiles;
}

void MainWindow::importGMS()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select GMS file to import", "","GMS files (*.txt)");
     if (!fileName.isEmpty()) loadFile(fileName);
}

void MainWindow::importGoCad()
{
    QString fileName = QFileDialog::getOpenFileName(this,
		"Select data file to import", "","Gocad files (*.ts);;Gocad lines (*.tline)");
     if (!fileName.isEmpty()) {
         loadFile(fileName);
     }
}

void MainWindow::importRaster()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Select raster file to import", "","Raster files (*.asc *.bmp *.jpg *.png *.tif);;");
	QFileInfo fi(fileName);

	if ((fi.suffix().toLower() == "asc") ||
		(fi.suffix().toLower() == "tif") ||
		(fi.suffix().toLower() == "png") ||
		(fi.suffix().toLower() == "jpg") ||
		(fi.suffix().toLower() == "bmp"))
	{
		QImage raster;
		QPointF origin;
		double scalingFactor;
		OGSRaster::loadImage(fileName, raster, origin, scalingFactor);

		VtkBGImageSource* bg = VtkBGImageSource::New();
			bg->SetOrigin(origin.x(), origin.y());
			bg->SetCellSize(scalingFactor);
			bg->SetRaster(raster);
		_vtkVisPipeline->addPipelineItem(bg);
	}
	else OGSError::box("File extension not supported.");
}

void MainWindow::importShape()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Select shape file to import", "","ESRI Shape files (*.shp );;");
//	QString fileName = QFileDialog::getOpenFileName(this, "Select shape file to import", "","ESRI Shape files (*.shp *.dbf);;");
	QFileInfo fi(fileName);

	if (fi.suffix().toLower() == "shp" || fi.suffix().toLower() == "dbf")
	{
		SHPImportDialog dlg( (fileName.toUtf8 ()).constData(), _geoModels);
		dlg.exec();
	}
}

void MainWindow::importPetrel()
{
    QStringList sfc_file_names = QFileDialog::getOpenFileNames(this,
		"Select surface data file(s) to import", "","Petrel files (*)");
    QStringList well_path_file_names = QFileDialog::getOpenFileNames(this,
    		"Select well path data file(s) to import", "","Petrel files (*)");
     if (sfc_file_names.size() != 0 || well_path_file_names.size() != 0 ) {
         loadPetrelFiles (sfc_file_names, well_path_file_names);
     }
}

void MainWindow::showPropertiesDialog(std::string name)
{
	ListPropertiesDialog dlg(name, _geoModels);
	connect(&dlg, SIGNAL(propertyBoundariesChanged(std::string, std::vector<PropertyBounds>)), _geoModels, SLOT(filterStationVec(std::string, std::vector<PropertyBounds>)));
	dlg.exec();
}

void MainWindow::showAddPipelineFilterItemDialog( QModelIndex parentIndex )
{
	VtkAddFilterDialog dlg(_vtkVisPipeline, parentIndex);
	dlg.exec();
}

void MainWindow::exportBoreholesToGMS(std::string listName, std::string fileName)
{
	const std::vector<GEOLIB::Point*> *stations (_geoModels->getStationVec(listName));
	GMSInterface::writeBoreholesToGMS(stations, fileName);
}

void MainWindow::showDiagramPrefsDialog(QModelIndex &index)
{
	QString listName;
	GEOLIB::Station* stn = _geoModels->getStationModel()->stationFromIndex(index, listName);

	if (stn->type() == GEOLIB::Station::STATION)
	{
		DiagramPrefsDialog* prefs = new DiagramPrefsDialog(stn, listName, _db);
		prefs->show();
	}
	if (stn->type() == GEOLIB::Station::BOREHOLE)
		OGSError::box("No time series data available for borehole.");
}

void MainWindow::showVisalizationPrefsDialog()
{
	VisPrefsDialog* visPrefs = new VisPrefsDialog(_vtkVisPipeline);
    visPrefs->show();
}


void MainWindow::ShowWindow()
{
	this->show();
}

void MainWindow::HideWindow()
{
	this->hide();
}

void MainWindow::on_actionExportVRML2_triggered( bool checked /*= false*/ )
{
	vtkVRMLExporter* exporter = vtkVRMLExporter::New();
	QString fileName = QFileDialog::getSaveFileName(this, "Save scene to VRML file", "","VRML files (*.wrl);;");
	exporter->SetFileName(fileName.toStdString().c_str());
	exporter->SetRenderWindow(visualizationWidget->vtkWidget->GetRenderWindow());
	exporter->Write();
	exporter->Delete();
}

void MainWindow::on_actionExportObj_triggered( bool checked /*= false*/ )
{
	vtkOBJExporter* exporter = vtkOBJExporter::New();
	QString fileName = QFileDialog::getSaveFileName(this, "Save scene to Wavefront OBJ files", "",";;");
	exporter->SetFilePrefix(fileName.toStdString().c_str());
	exporter->SetRenderWindow(visualizationWidget->vtkWidget->GetRenderWindow());
	exporter->Write();
	exporter->Delete();
}

void MainWindow::showVisDockWidget( bool show )
{
	show ? vtkVisTabWidget->show() : vtkVisTabWidget->hide();
}