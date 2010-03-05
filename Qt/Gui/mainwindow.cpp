/**
* \file mainwindow.h
* 4/11/2009 LB Initial implementation
*
*/

#include "mainwindow.h"
#include "msh_lib.h"
#include "GraphicsScene.h"
#include "GraphicsView2d.h"
#include "PntsModel.h"
#include "LinesModel.h"
#include "StationTreeModel.h"
#include "MshModel.h"
#include "ViewWidget2d.h"
#include "OGSError.h"
#include "DBConnectionDialog.h"
#include "DiagramPrefsDialog.h"
#include "SHPImportDialog.h"
#include "OGSRaster.h"
#include "GEOModels.h"
#include "ListPropertiesDialog.h"
#include "Configure.h"
#include "VtkVisPipeline.h"
#include "GraphicsItem2d.h"
#include "VtkAddFilterDialog.h"

// GEOLIB includes
#include "Point.h"
#include "Polyline.h"
#include "Station.h"

// FileIO includes
#include "OGSIOVer4.h"
#include "PetrelInterface.h"
#include "GocadInterface.h"
//#include "XMLInterface.h"

#include "RecentFiles.h"


#include <QFileDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QSettings>
#include <QComboBox>
#include <QPixmap>

using namespace FileIO;


MainWindow::MainWindow(QWidget *parent /* = 0*/)
: QMainWindow(parent), _db (NULL)
{
    setupUi(this);


	// Setup connection GEOObjects to GUI through GEOModels and tab widgets
	_geoModels = new GEOModels();


	// Get graphics scene
	GraphicsScene* scene = visualizationWidget->scene();
	connect(scene, SIGNAL(sceneChanged()), this, SLOT(updateGraphicsScene()));

	// station model
	stationTabWidget->treeView->setModel(_geoModels->getStationModel());
	connect(_geoModels, SIGNAL(stationVectorAdded(StationTreeModel*, std::string)),
		scene, SLOT(loadItemsFromTreeModel(StationTreeModel*, std::string)));		// update model when stations are added
	connect(stationTabWidget->treeView, SIGNAL(stationListRemoved(std::string)),
		_geoModels, SLOT(removeStationVec(std::string)));	// update model when stations are removed
	connect(_geoModels, SIGNAL(stationVectorRemoved(StationTreeModel*, std::string)),
		this, SLOT(updateGraphicsScene()));					// update 2d view when stations are removed

	// point models
	connect (_geoModels, SIGNAL(pointModelAdded(Model*)),
		pntTabWidget->dataViewWidget, SLOT(addModel(Model*)));
	connect(pntTabWidget->dataViewWidget, SIGNAL(requestModelClear(std::string)),
		_geoModels, SLOT(removePointVec(const std::string)));
	connect (_geoModels, SIGNAL(pointModelRemoved(Model*)),
		pntTabWidget->dataViewWidget, SLOT(removeModel(Model*)));
	connect(_geoModels, SIGNAL(pointModelRemoved(Model*)),
		this, SLOT(updateGraphicsScene()));
	connect(_geoModels, SIGNAL(pointModelAdded(Model*)),
		scene, SLOT(loadItemsFromTableModel(Model*)));

	// line models
	connect (_geoModels, SIGNAL(polylineModelAdded(Model*)),
		lineTabWidget->dataViewWidget, SLOT(addModel(Model*)));
	connect(lineTabWidget->dataViewWidget, SIGNAL(requestModelClear(std::string)),
		_geoModels, SLOT(removePolylineVec(const std::string)));
	connect (_geoModels, SIGNAL(polylineModelRemoved(Model*)),
		lineTabWidget->dataViewWidget, SLOT(removeModel(Model*)));
	connect(_geoModels, SIGNAL(polylineModelRemoved(Model*)),
		this, SLOT(updateGraphicsScene()));
	connect(_geoModels, SIGNAL(polylineModelAdded(Model*)),
		scene, SLOT(loadItemsFromTableModel(Model*)));

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
	connect(_geoModels, SIGNAL(stationVectorAdded(StationTreeModel*, std::string)),
		_vtkVisPipeline, SLOT(addPipelineItem(StationTreeModel*, std::string)));
	connect(_geoModels, SIGNAL(stationVectorRemoved(StationTreeModel*, std::string)),
		_vtkVisPipeline, SLOT(removeSourceItem(StationTreeModel*, std::string)));

	connect(_vtkVisPipeline, SIGNAL(vtkVisPipelineChanged()),
		visualizationWidget->vtkWidget, SLOT(update()));

	vtkVisTabWidget->treeView->setModel(_vtkVisPipeline);
	connect(vtkVisTabWidget->treeView, SIGNAL(requestRemovePipelineItem(QModelIndex)),
		_vtkVisPipeline, SLOT(removePipelineItem(QModelIndex)));
	connect(vtkVisTabWidget->treeView, SIGNAL(requestAddPipelineFilterItem(QModelIndex)),
		this, SLOT(showAddPipelineFilterItemDialog(QModelIndex)));


	// Stack the data dock widgets together
	tabifyDockWidget(pntDock, lineDock);
	tabifyDockWidget(lineDock, stationDock);
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

	QAction* showMshDockAction = mshDock->toggleViewAction();
	showMshDockAction->setStatusTip(tr("Shows / hides the lines view"));
	connect(showMshDockAction, SIGNAL(triggered(bool)), this, SLOT(showMshDockWidget(bool)));
	menuWindows->addAction(showMshDockAction);

	// Edit mode combo box
	connect(visualizationWidget->viewWidget2d->itemTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setConnectsSceneToTabWidgets(int)));
	setConnectsSceneToTabWidgets(visualizationWidget->viewWidget2d->itemTypeComboBox->currentIndex());

	// connects for point model
	//connect(pntTabWidget->pointsTableView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
	//	pntsModel, SLOT(setSelectionFromOutside(const QItemSelection&, const QItemSelection&)));
	//connect(pntTabWidget->clearAllPushButton, SIGNAL(clicked()), pntsModel, SLOT(clearData()));
	//connect(pntTabWidget->clearSelectedPushButton, SIGNAL(clicked()), pntsModel, SLOT(clearSelectedData()));
	//connect(pntTabWidget->clearAllPushButton, SIGNAL(clicked()), linesModel, SLOT(clearData()));
	//connect(pntsModel, SIGNAL(selectionCleared()), pntTabWidget->pointsTableView, SLOT(clearSelection()));

	// connects for lines model
	//connect(lineTabWidget->linesTableView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
	//	linesModel, SLOT(setSelectionFromOutside(const QItemSelection&, const QItemSelection&)));

	// connects for station model
	//connect(stationTabWidget->treeView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
	//	_stationModel, SLOT(setSelectionFromOutside(const QItemSelection&, const QItemSelection&)));
	//connect(_stationModel, SIGNAL(updateScene()), this, SLOT(updateGraphicsScene()));
	connect(stationTabWidget->treeView, SIGNAL(propertiesDialogRequested(std::string)), this, SLOT(showPropertiesDialog(std::string)));


	mshTabWidget->setScene(scene);
	//connect(mshTabWidget->mshTableView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
	//	mshModel, SLOT(setSelectionFromOutside(const QItemSelection&, const QItemSelection&)));
	//connect(mshTabWidget->clearAllPushButton, SIGNAL(clicked()), mshModel, SLOT(clearData()));
	//connect(mshTabWidget->clearSelectedPushButton, SIGNAL(clicked()), mshModel, SLOT(clearSelectedData()));
	connect(mshTabWidget->showMeshesCheckBox, SIGNAL(toggled(bool)), scene, SLOT(toggleMshVisibility(bool)));
	connect(mshTabWidget->showMeshNodesCheckBox, SIGNAL(toggled(bool)), scene, SLOT(toggleMshNodeVisibility(bool)));
	connect(mshTabWidget->showMeshElementsCheckBox, SIGNAL(toggled(bool)), scene, SLOT(toggleMshElemVisibility(bool)));
	connect(pntTabWidget->dataViewWidget->visibleCheckBox, SIGNAL(toggled(bool)), scene, SLOT(togglePntsVisibility(bool)));
	connect(lineTabWidget->dataViewWidget->visibleCheckBox, SIGNAL(toggled(bool)), scene, SLOT(toggleLinesVisibility(bool)));
	connect(stationTabWidget->showStationsCheckBox, SIGNAL(toggled(bool)), scene, SLOT(toggleStationsVisibility(bool)));

	std::cout << "size of Point: " << sizeof (GEOLIB::Point) << std::endl;
	std::cout << "size of CGLPoint: " << sizeof (CGLPoint) << std::endl;

	std::cout << "size of Polyline: " << sizeof (GEOLIB::Polyline) << std::endl;
	std::cout << "size of CGLPolyline: " << sizeof (CGLPolyline) << std::endl;

	std::cout << "size of GEOLIB::Surface: " << sizeof (GEOLIB::Surface) << std::endl;
	std::cout << "size of Surface: " << sizeof (Surface) << std::endl;

	std::cout << "size of CCore: " << sizeof (Mesh_Group::CCore) << std::endl;
	std::cout << "size of CNode: " << sizeof (Mesh_Group::CNode) << std::endl;
	std::cout << "size of CElement: " << sizeof (Mesh_Group::CNode) << std::endl;
	std::cout << "size of CEdge: " << sizeof (Mesh_Group::CEdge) << std::endl;
	std::cout << "size of CFEMesh: " << sizeof (Mesh_Group::CFEMesh) << std::endl;
	std::cout << "size of Matrix: " << sizeof (Math_Group::Matrix) << std::endl;
}

MainWindow::~MainWindow()
{
	if (_db) delete _db;
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

void MainWindow::showMshDockWidget( bool show )
{
	if (show)
		mshDock->show();
	else
		mshDock->hide();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
		"Select data file to open", "","Geosys files (*.gli *.gml *.msh *.stn);;GLI files (*.gli);;MSH files (*.msh);;STN files (*.stn);;All files (* *.*)");
     if (!fileName.isEmpty()) {
         loadFile(fileName);
     }
}


void MainWindow::openDatabase()
{
	if (_db==NULL)
	{
		_db = new DatabaseConnection(_geoModels);
		//connect(_db, SIGNAL(listLoaded(QString)), _stationModel, SLOT(addStationList(QString)));
		_db->dbConnect();
	}

	if (_db!=NULL && _db->isConnected())
	{
		_db->getListSelection();
		updateGraphicsScene();
		visualizationWidget->viewWidget2d->setEditableItemType();
		setConnectsStationsToDiagramView();
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
    QString fileName = QFileDialog::getSaveFileName(this, "Save data as", "","Geosys XML files (*.gml);;All files (* *.*)");

	QString gliName = pntTabWidget->dataViewWidget->modelSelectComboBox->currentText();

	if (!fileName.isEmpty())
	{
		QFile* file = new QFile(fileName);
		file->open( QIODevice::WriteOnly );

//		std::string schemaName(SOURCEPATH);
//		schemaName.append("/OpenGeoSysGLI.xsd");
//		XMLInterface xml(_geoModels, schemaName);
//		xml.writeGLIFile(file, gliName);
		file->close();

//		xml.insertStyleFileDefinition(fileName);
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

		if (int returnValue = _geoModels->readStationFile(fileName.toStdString(), name, stations, type))
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

	updateGraphicsScene();
	setConnectsStationsToDiagramView();

	visualizationWidget->viewWidget2d->setEditableItemType();
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

void MainWindow::updateGraphicsScene()
{
	GraphicsScene* scene = visualizationWidget->scene();

	QRectF rect = scene->itemsBoundingRect();
	scene->setSceneRect(rect);
	// TODO emit itemsLoaded();

	pntTabWidget->dataViewWidget->dataView->resizeColumnsToContents();
	pntTabWidget->dataViewWidget->dataView->resizeRowsToContents();
	lineTabWidget->dataViewWidget->dataView->resizeColumnsToContents();
	lineTabWidget->dataViewWidget->dataView->resizeRowsToContents();
	mshTabWidget->mshTableView->resizeColumnsToContents();
	mshTabWidget->mshTableView->resizeRowsToContents();
	visualizationWidget->showAll();

    QApplication::restoreOverrideCursor();
}


void MainWindow::readSettings()
{
	QSettings settings("UFZ", "OpenGeoSys-5");

	restoreGeometry(settings.value("windowGeometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	visualizationWidget->tabWidget->setCurrentIndex(
		settings.value("visWidgetActiveTab", 2).toInt());
}

void MainWindow::writeSettings()
{
	QSettings settings("UFZ", "OpenGeoSys-5");

	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("visWidgetActiveTab",
		visualizationWidget->tabWidget->currentIndex());
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About OpenGeoSys-5"), tr("Built on %1").
		arg(QDate::currentDate().toString()));
}

void MainWindow::setConnectsSceneToTabWidgets( int type )
{
	GraphicsScene* scene = visualizationWidget->scene();
	Model* model;

	disconnect(pntTabWidget->dataViewWidget->dataView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
	disconnect(lineTabWidget->dataViewWidget->dataView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
	disconnect(mshTabWidget->mshNodeTableView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));

	disconnect((Model*)pntTabWidget->dataViewWidget->dataView->model(), SLOT(setSelectionFromOutside( const QItemSelection&, const QItemSelection&)));
	disconnect((Model*)lineTabWidget->dataViewWidget->dataView->model(), SLOT(setSelectionFromOutside( const QItemSelection&, const QItemSelection&)));
	disconnect((Model*)stationTabWidget->treeView->model(), SLOT(setSelectionFromOutside( const QItemSelection&, const QItemSelection&)));
	disconnect((Model*)mshTabWidget->mshTableView->model(), SLOT(setSelectionFromOutside( const QItemSelection&, const QItemSelection&)));

	switch (type)
	{
	case 0:
		connect(scene, SIGNAL(itemSelectionChanged( const QItemSelection&, const QItemSelection&)),
				pntTabWidget->dataViewWidget->dataView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
		model = (Model*)pntTabWidget->dataViewWidget->dataView->model();
		//connect(scene, SIGNAL(itemSelectionChanged( const QItemSelection&, const QItemSelection&)),
		//	model, SLOT(setSelectionFromOutside( const QItemSelection&, const QItemSelection&)));
		//connect(model, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		//	pntTabWidget->pointsTableView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));

		break;
	case 1: connect(visualizationWidget->scene(), SIGNAL(itemSelectionChanged( const QItemSelection&, const QItemSelection&)),
				lineTabWidget->dataViewWidget->dataView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
		break;
	case 2: connect(visualizationWidget->scene(), SIGNAL(itemSelectionChanged( const QItemSelection&, const QItemSelection&)),
				stationTabWidget->treeView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
		break;
	case 3: connect(visualizationWidget->scene(), SIGNAL(itemSelectionChanged( const QItemSelection&, const QItemSelection&)),
				mshTabWidget->mshNodeTableView, SLOT(selectionChangedFromOutside( const QItemSelection&, const QItemSelection&)));
		break;
	}
}

void MainWindow::setConnectsStationsToDiagramView()
{
	foreach (QGraphicsItem* g, visualizationWidget->scene()->items())
	{
		GraphicsItem2d* item = static_cast<GraphicsItem2d*>(g);
		if (item->itemType() == STATION_ITEM)
		{
			connect(item,SIGNAL(diagramRequested(QModelIndex&)),this, SLOT(showDiagramPrefs(QModelIndex&)));
		}
	}

}

void MainWindow::showDiagramPrefs(QModelIndex &idx)
{
	QString listName;
	GEOLIB::Station* stn = _geoModels->getStationModel()->stationFromIndex(idx, listName);

	if (stn->type() == GEOLIB::Station::STATION)
	{
		DiagramPrefsDialog* prefs = new DiagramPrefsDialog(stn, listName, _db);
		prefs->show();
	}
	if (stn->type() == GEOLIB::Station::BOREHOLE)
		OGSError::box("No time series data available for borehole.");
}


QMenu* MainWindow::createImportFilesMenu()
{
	QMenu* importFiles = new QMenu("Import Files");
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
		QPixmap raster;
		QPointF origin;
		double scalingFactor;
		GraphicsScene* scene = visualizationWidget->scene();
		OGSRaster::loadPixmap(fileName, raster, origin, scalingFactor);
		scene->drawPixmap(raster, origin, scalingFactor);
		updateGraphicsScene();
		emit fileUsed(fileName);
	}
	else OGSError::box("File extension not supported.");
}

void MainWindow::importShape()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Select shape file to import", "","ESRI Shape files (*.shp);;");
	QFileInfo fi(fileName);

	if (fi.suffix().toLower() == "shp")
	{
		SHPImportDialog dlg(fileName.toStdString(), _geoModels);
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
