cmake_minimum_required(VERSION 2.6)

find_package( GTest REQUIRED )

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIR})
  enable_testing()
  add_definitions( -DOGS_EXECUTABLE="\\"${PROJECT_BINARY_DIR}/bin/ogs\\"" )
  add_executable( test_ogs testOgs.cxx )

  set_target_properties ( test_ogs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests )

  target_link_libraries( test_ogs ${GTEST_BOTH_LIBRARIES} )
  GTEST_ADD_TESTS( test_ogs "" testOgs.cxx )

endif()
