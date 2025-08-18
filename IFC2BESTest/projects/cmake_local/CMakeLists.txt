# CMakeLists.txt file for IFC2BESTest

project( IFC2BESTest )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../externals/IFCImportPlugin/src
	${PROJECT_SOURCE_DIR}/../../../externals/GEGExportPlugin/src
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
)

# gather all cpp files in IFC2BESTest directory
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# this will run uic on .ui files:
qt5_wrap_ui( APP_UI_SRCS ${APP_UIS} )

# and finally this will run moc:
qt5_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

# generate list of all required QT_LIBRARIES
set ( QT_LIBRARIES 
	Qt5::Widgets
	Qt5::PrintSupport
	Qt5::Svg
	Qt5::Xml
)

# set variable for dependent libraries
set( LINK_LIBS
	IBKMK
	IBK
	TiCPP
)


# build application executable for the different platforms
if( WIN32 )
	add_executable( ${PROJECT_NAME} WIN32 
					 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_UI_SRCS}
	)

	set(WIN_FRAMEWORKS iphlpapi EmfEngine gdiplus gdi32 user32 ole32)

endif( WIN32 ) 

if( UNIX )
	if( APPLE )
		add_executable( ${PROJECT_NAME} MACOSX_BUNDLE 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_UI_SRCS} 
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_UI_SRCS} 
		)
	endif( APPLE ) 
endif( UNIX )

# and link it against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS}
	${QT_LIBRARIES}
)
