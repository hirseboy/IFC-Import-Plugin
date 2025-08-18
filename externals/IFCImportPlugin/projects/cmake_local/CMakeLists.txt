cmake_minimum_required(VERSION 3.16)

project(ImportIFCPlugin)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatisierung für Qt MOC, UIC, RCC
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)

# Plugin-spezifisches Makro
add_definitions(-DQT_PLUGIN)

# Plugin-Ausgabe-Verzeichnis (analog zu DESTDIR/DLLDESTDIR)
# set(PLUGIN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../../bin/release)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})

# Include-Verzeichnisse
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${PROJECT_SOURCE_DIR}/../../../IBK/src
	${PROJECT_SOURCE_DIR}/../../../IBKMK/src
	${PROJECT_SOURCE_DIR}/../../../TiCPP/src
	${PROJECT_SOURCE_DIR}/../../../QtExt/src
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/IFC4X3/include
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/include
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/external
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/external/glm
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/external/manifold/src/utilities/include
	${PROJECT_SOURCE_DIR}/../../../ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/common
	${PROJECT_SOURCE_DIR}/../../../IFCConvert/src
)

# Qt Module
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# Quellen, Header, Forms
set(SRC_FILES
    ${PROJECT_SOURCE_DIR}/../../src/IFCImportPlugin.cpp
    ${PROJECT_SOURCE_DIR}/../../src/ImportIFCDialog.cpp
    ${PROJECT_SOURCE_DIR}/../../src/ImportIFCMessageHandler.cpp
)

set(LIB_HDRS
    ${PROJECT_SOURCE_DIR}/../../src/ImportIFCDialog.h
    ${PROJECT_SOURCE_DIR}/../../src/ImportIFCMessageHandler.h
#    ${PROJECT_SOURCE_DIR}/../../src/SVCommonPluginInterface.h
#    ${PROJECT_SOURCE_DIR}/../../src/SVImportPluginInterface.h
    ${PROJECT_SOURCE_DIR}/../../src/IFCImportPlugin.h
)

set(LIB_UIS
    ${PROJECT_SOURCE_DIR}/../../src/ImportIFCDialog.ui
)

set(METADATA_JSON_FILE ${PROJECT_SOURCE_DIR}/../../src/metadata.json)

qt5_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )
qt5_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

# Übersetzungen
# set(TS_FILES resources/translations/ImportIFCPlugin_de.ts)
# qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# Plugin-Bibliothek
add_library(ImportIFCPlugin SHARED
    ${SRC_FILES}
    ${LIB_MOC_SRCS}
    ${LIB_UI_SRCS}
#    ${QM_FILES}
	${METADATA_JSON_FILE}
)

# Weitere statische Bibliotheken (analog zu LIBS += ...)
target_link_libraries(ImportIFCPlugin PRIVATE
    IFCConvert_static
    QtExt_static
    ifcplusplus_static
    clipper_static
    Carve_static
    TiCPP_static
    IBKMK_static
    IBK_static
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Svg
    Qt5::PrintSupport
)

# Optional: Windows-spezifische Libraries
if(WIN32)
    target_link_libraries(ImportIFCPlugin PRIVATE iphlpapi shell32)
endif()

# Export-Ziel (analog zu INSTALLS += target)
#  install(TARGETS ImportIFCPlugin
#      RUNTIME DESTINATION PLUGIN_OUTPUT_DIR
#      LIBRARY DESTINATION PLUGIN_OUTPUT_DIR
#  )
