cmake_minimum_required(VERSION 2.6)

find_package( GTest REQUIRED )

if(GTEST_FOUND)
  include_directories( ${GTEST_INCLUDE_DIR} )
  enable_testing()
  add_executable( test_bmskel testBmSkel.cxx )

  set_target_properties ( test_bmskel PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests )

  target_link_libraries( test_bmskel ${GTEST_BOTH_LIBRARIES} )
  gtest_add_tests( test_bmskel "" testBmSkel.cxx )

  # copies the model files to the build dir and adds them as targets so that
  # the build files are re-built when the source model files change
  macro ( UPDATE_MODEL_FILES dirOUT fileLIST )
    get_filename_component( _tdir ${CMAKE_CURRENT_SOURCE_DIR} NAME )
    #message (STATUS "Copying files to ${dirOUT} from ${fileLIST}.\n")
    foreach ( _file1 ${${fileLIST}} )
      set( _file ${CMAKE_CURRENT_SOURCE_DIR}/${_file1} )
      get_filename_component( _fdest ${_file} NAME )
      set( dest ${dirOUT}/${_fdest} )
      #message( STATUS "Copying ${_file} to ${dest} \n" )

    #message (STATUS "Adding targets ${_tdir}.${_fdest}\n").\n")
      add_custom_target( ${_tdir}.${_fdest}
	${CMAKE_COMMAND} -E copy_if_different
	${_file} ${dest})

      add_dependencies( test_bmskel ${_tdir}.${_fdest} )
    endforeach(_file1 ${${fileLIST}})

  endmacro ( UPDATE_MODEL_FILES dirOUT fileLIST)

  # add new test models here not all of these models will be needed, depending
  # on compile flags but it is safe to copy them all across
  add_subdirectory( data/bmskel )

endif()
