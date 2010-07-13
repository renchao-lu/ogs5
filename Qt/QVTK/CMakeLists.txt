
SET(DESIRED_QT_VERSION 4)

SET ( QT_USE_QTSQL 1 )

# set up sources to build
SET ( QVTKLibSrcs
    vtkEventQtSlotConnect.cxx
    vtkQtConnection.cxx
    QVTKWidget.cxx
    )

SET ( QVTKMocHeaders
    ${CMAKE_CURRENT_BINARY_DIR}/QVTKWidget.h 
    vtkQtConnection.h
    )

INCLUDE_DIRECTORIES (
    .
    ${CMAKE_CURRENT_BINARY_DIR} )


  SET(Q_SIGNALS Q_SIGNALS)
  SET(Q_SLOTS Q_SLOTS)

  # add files that require Qt 4
  SET ( QVTKLibSrcs
      ${QVTKLibSrcs}
      vtkQtAbstractModelAdapter.cxx
      vtkQtInitialization.cxx
      vtkQtSQLDatabase.cxx
      vtkQtSQLQuery.cxx
      vtkQtTableModelAdapter.cxx
      vtkQtTimePointUtility.cxx
      vtkQtTreeModelAdapter.cxx 
      )
    
  SET ( QVTKMocHeaders
      ${QVTKMocHeaders}
      vtkQtAbstractModelAdapter.h
      vtkQtTableModelAdapter.h
      vtkQtTreeModelAdapter.h
      )

  # add additional files depending on infovis and/or views
  #IF(VTK_USE_VIEWS)
  #  SET( QVTKLibSrcs ${QVTKLibSrcs}
  #    vtkQtChartRepresentation.cxx
  #    vtkQtChartTableRepresentation.cxx
  #    vtkQtChartView.cxx
  #    vtkQtChartViewBase.cxx
  #    vtkQtBarChartView.cxx
  #    vtkQtLineChartView.cxx
  #    vtkQtStackedChartView.cxx
  #    vtkQtStatisticalBoxChartView.cxx
  #    vtkQtItemView.cxx
  #    vtkQtListView.cxx
  #    vtkQtTableDataRepresentation.cxx
  #    vtkQtTableMetadataRepresentation.cxx
  #    vtkQtTableRepresentation.cxx
  #    vtkQtTableView.cxx
  #    vtkQtTreeView.cxx
  #    vtkQtView.cxx
  #    ) 
  #  SET ( QVTKMocHeaders ${QVTKMocHeaders}
  #    vtkQtItemView.h
  #    vtkQtChartRepresentation.h
  #    )
  #ENDIF(VTK_USE_VIEWS)
  
  # help CMake fine QtDesigner on Mac (binary install)
  FIND_PATH(QT_QTDESIGNER_INCLUDE_DIR QDesignerComponents
            PATHS ${QT_LIBRARY_DIR}/QtDesigner.framework/Headers
            NO_DEFAULT_PATH)

  # import Qt4 build settings
  INCLUDE(${QT_USE_FILE})

  SET ( PluginLibSrcs
        Q4VTKWidgetPlugin.cxx
        )

  SET ( PluginMocHeaders
        Q4VTKWidgetPlugin.h
        )

  # CMake apparently doesn't do this automatically
  SET_SOURCE_FILES_PROPERTIES(Q4VTKWidgetPlugin.cxx PROPERTIES 
                              OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/QVTKWidget.xpm)

  # Reduce the number of dirs that get included on moc command line
  # since it causes issues on Windows 2000.
  GET_DIRECTORY_PROPERTY(include_dirs_tmp INCLUDE_DIRECTORIES)
  SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${MOC_INCLUDE_DIRS}")

  QT4_WRAP_CPP ( QVTKLibMocSrcs ${QVTKMocHeaders} )

  SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${include_dirs_tmp}")

  SET(QT_LIBS "${QT_LIBRARIES}")


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/QVTKWidget.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/QVTKWidget.h
               @ONLY)

ADD_LIBRARY(QVTK ${QVTKLibSrcs} ${QVTKLibMocSrcs})

# Apply user-defined properties to the library target.
IF(VTK_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(QVTK PROPERTIES ${VTK_LIBRARY_PROPERTIES})
ENDIF(VTK_LIBRARY_PROPERTIES)

TARGET_LINK_LIBRARIES( QVTK 
  ${QT_LIBS}
  vtkRendering 
  vtkGraphics
  vtkImaging 
  vtkCommon)

# add extra dependencies if necessary
#TARGET_LINK_LIBRARIES( QVTK vtkQtChart )
IF(VTK_USE_VIEWS)
  TARGET_LINK_LIBRARIES( QVTK vtkViews )
ENDIF(VTK_USE_VIEWS)
