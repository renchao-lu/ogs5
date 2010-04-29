SET ( SOURCES
	testrunner.cpp
	Test_Base.cpp
)

include_directories(
	../Base
)

add_executable (testrunner ${SOURCES})
target_link_libraries(testrunner ${GTEST_BOTH_LIBRARIES})

IF (WIN32)
	SET (testrunnerExe ${EXECUTABLE_OUTPUT_PATH}/Release/testrunner)
ELSE (WIN32)
	SET (testrunnerExe ${EXECUTABLE_OUTPUT_PATH}/testrunner)
ENDIF(WIN32)

add_test (AllTests ${testrunnerExe})

macro(ADD_GOOGLE_TESTS executable)
  foreach ( source ${ARGN} )
    file(READ "${source}" contents)
    string(REGEX MATCHALL "TEST_?F?\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
    foreach(hit ${found_tests})
      string(REGEX REPLACE ".*\\(([A-Za-z_0-9]+)[, ]*([A-Za-z_0-9]+)\\).*" "\\1.\\2" test_name ${hit})
      add_test(${test_name} ${executable} --gtest_filter=${test_name} ${MI3CTestingDir})
      message ("Adding GTest")
    endforeach(hit)
  endforeach()
endmacro()

# Add all tests found in the source code, calling the executable to run them
#ADD_GOOGLE_TESTS ( ${testrunnerExe} ${SOUCRES})

