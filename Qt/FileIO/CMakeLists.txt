# 11/08/2010 LB Initial implementation
# CMake file for QtFileIO

### Source files ###
SET( SOURCES
	WriterBase.cpp
)

### Header files ###
SET( HEADERS
	WriterBase.h
)

### Include directories ###
INCLUDE_DIRECTORIES(
	.
)

### Create the library ###
ADD_LIBRARY( QtFileIO STATIC
	${SOURCES}
	${HEADERS}
)

### Link other libraries ###
TARGET_LINK_LIBRARIES( QtFileIO
	VtkVis
)
