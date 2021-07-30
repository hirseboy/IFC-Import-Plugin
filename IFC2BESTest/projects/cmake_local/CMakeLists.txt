# CMakeLists.txt file for IFC2BESTest

project( IFC2BESTest )

if (CMAKE_COMPILER_IS_GNUCC)
	if (NOT APPLE)
		# link solver statically
		set( CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++" )
	endif (NOT APPLE)
endif (CMAKE_COMPILER_IS_GNUCC)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
)

# gather all cpp files in IFC2BESTest directory
file( GLOB IFC2BESTest_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# set variable for dependent libraries
set( LINK_LIBS
	IBKMK
	IBK
)

# now build the IFC2BESTest executable - this only requires compiling the main.cpp
add_executable( ${PROJECT_NAME}
	${IFC2BESTest_SRCS}
)

# and link it against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
)
