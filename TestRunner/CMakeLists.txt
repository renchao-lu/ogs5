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

macro(ADD_GOOGLE_TESTS executable)
	foreach ( source ${ARGN} )
		file(READ "${source}" contents)
		string(REGEX MATCHALL "TEST_?F?\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
		foreach(hit ${found_tests})
			string(REGEX REPLACE ".*\\(([A-Za-z_0-9]+)[, ]*([A-Za-z_0-9]+)\\).*" "\\1.\\2" test_name ${hit})
			add_test(${test_name} ${executable} --gtest_filter=${test_name} ${MI3CTestingDir})
			# message ("Adding test: ${test_name}")
		endforeach(hit)
	endforeach()
endmacro()

# Either collect all gtests in one ctest run of testrunner executable
# or run each gtest individually
IF (OGS_ONE_BIG_GTEST)
	add_test (AllTests ${testrunnerExe})
ELSE (OGS_ONE_BIG_GTEST)
	ADD_GOOGLE_TESTS ( ${testrunnerExe} ${SOURCES})
ENDIF (OGS_ONE_BIG_GTEST)

