# CMakeLists.txt file for ifcplusplus library

project( ifcplusplus )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
)

# collect a list of all source files of the IBK library
file( GLOB LIB_SRCS 
  ${PROJECT_SOURCE_DIR}/../../src/*.cpp 
)
    
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj ")
endif()

add_definitions(-DIFCQUERY_AS_DYNAMIC_LIBRARY)
add_definitions(-DIFCQUERY_LIB)

# build the library
add_library( ifcplusplus SHARED
	${LIB_SRCS}
)

