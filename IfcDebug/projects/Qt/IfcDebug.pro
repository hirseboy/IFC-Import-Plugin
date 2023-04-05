# Project file for IfcDebug
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = IfcDebug
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
#include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4) {
	contains(QT_ARCH, i386): {
		DIR_PREFIX =
	} else {
		DIR_PREFIX = _x64
	}
} else {
	DIR_PREFIX =
}

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug$${DIR_PREFIX}
	DESTDIR = ../../../bin/debug$${DIR_PREFIX}
}
else {
	OBJECTS_DIR = release$${DIR_PREFIX}
	DESTDIR = ../../../bin/release$${DIR_PREFIX}
}

MOC_DIR = moc
UI_DIR = ui

win32-msvc* {
	QMAKE_CXXFLAGS += /wd4996
	QMAKE_CFLAGS += /wd4996
	DEFINES += _CRT_SECURE_NO_WARNINGS
	DEFINES += NOMINMAX
}
else {
	QMAKE_CXXFLAGS += -std=c++11
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L../../../externals/lib_x64

LIBS += \
    -lCarve \
    -lImportIFCPlugin \
    -lIFCConvert \
    -lclipper \
    -lifcplusplus \
    -lQtExt \
    -lTiCPP \
    -lIBKMK \
    -lIBK

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/TiCPP/src \
        ../../../externals/IFCConvert/src \
        ../../../externals/clipper/src \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/ifcpp/IFC4X3/include \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/include \
        ../../../externals/ifcplusplus/src/external \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/external \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/external/glm \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/external/manifold/src/utilities/include \
        ../../../externals/ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/common \
        ../../../externals/IFCConvert/src \
	../../../externals/IFCImportPlugin/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/IfcDebug_MainWidget.cpp \
	../../src/main.cpp

HEADERS += \
    ../../src/IfcDebug_MainWidget.h


FORMS += \
    ../../src/IfcDebug_MainWidget.ui


TRANSLATIONS += ../../resources/translations/IfcDebug_de.ts
CODECFORSRC = UTF-8

