# CMakeLists.txt file for Carve library

project(Carve)

# collect a list of all source files of the library
file (GLOB LIB_SRCS 
	${PROJECT_SOURCE_DIR}/src/common/geometry.cpp
	${PROJECT_SOURCE_DIR}/src/lib/aabb.cpp
	${PROJECT_SOURCE_DIR}/src/lib/carve.cpp
	${PROJECT_SOURCE_DIR}/src/lib/convex_hull.cpp
	${PROJECT_SOURCE_DIR}/src/lib/csg.cpp
	${PROJECT_SOURCE_DIR}/src/lib/csg_collector.cpp
	${PROJECT_SOURCE_DIR}/src/lib/edge.cpp
	${PROJECT_SOURCE_DIR}/src/lib/face.cpp
	${PROJECT_SOURCE_DIR}/src/lib/geom.cpp
	${PROJECT_SOURCE_DIR}/src/lib/geom2d.cpp
	${PROJECT_SOURCE_DIR}/src/lib/geom3d.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_classify_edge.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_classify_group.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_debug.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_face_division.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_group.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersect_half_classify_group.cpp
	${PROJECT_SOURCE_DIR}/src/lib/intersection.cpp
	${PROJECT_SOURCE_DIR}/src/lib/math.cpp
	${PROJECT_SOURCE_DIR}/src/lib/mesh.cpp
	${PROJECT_SOURCE_DIR}/src/lib/octree.cpp
	${PROJECT_SOURCE_DIR}/src/lib/pointset.cpp
	${PROJECT_SOURCE_DIR}/src/lib/polyhedron.cpp
	${PROJECT_SOURCE_DIR}/src/lib/polyline.cpp
	${PROJECT_SOURCE_DIR}/src/lib/shewchuk_predicates.cpp
	${PROJECT_SOURCE_DIR}/src/lib/tag.cpp
	${PROJECT_SOURCE_DIR}/src/lib/timing.cpp
	${PROJECT_SOURCE_DIR}/src/lib/triangle_intersection.cpp
	${PROJECT_SOURCE_DIR}/src/lib/triangulator.cpp
)

SET( LIB_SRCS ${LIB_SRCS} )

include_directories(
${PROJECT_SOURCE_DIR}/src/include
${PROJECT_SOURCE_DIR}/src/common
)

# build the library
if (NOT MSVC)
	add_library( ${PROJECT_NAME} SHARED ${LIB_SRCS} )
	add_library( ${PROJECT_NAME}_static STATIC ${LIB_SRCS} )
else()
	add_library( ${PROJECT_NAME} STATIC ${LIB_SRCS} )
	add_library( ${PROJECT_NAME}_static ALIAS ${PROJECT_NAME} )
endif()
