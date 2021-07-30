# CMakeLists.txt file for ifcplusplus library

project( ifcplusplus )

# collect a list of all source files of the IBK library
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/src/ifcpp/IFC4/*.cpp )

# build the library
add_library( IBK STATIC
	${LIB_SRCS}
)

