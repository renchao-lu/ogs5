# Source files
set( SOURCES
	geo_dom.cpp
	geo_lib.cpp
	geo_lin.cpp
	geo_mathlib.cpp
	geo_ply.cpp
	geo_pnt.cpp
	geo_sfc.cpp
	geo_vol.cpp
	Station.cpp
	GEOObjects.cpp
)

# Header files
set( HEADERS
	Color.h
	geo_dom.h
	geo_lib.h
	geo_lin.h
	geo_mathlib.h
	geo_ply.h 	
	geo_pnt.h	
	geo_sfc.h		
	geo_vol.h	
	TemplatePoint.h
	Point.h
	Polyline.h
	Surface.h
	PointVec.h
	TemplateVec.h
	PolylineVec.h
	SurfaceVec.h
	PropertyBounds.h
	Station.h	
	GEOObjects.h
	Vector3.h
)

# Create the library
add_library( GEO STATIC
	${SOURCES}
	${HEADERS}
)


include_directories(
	../Base
	.
	../FileIO
)


target_link_libraries (
	GEO
	Base
)

