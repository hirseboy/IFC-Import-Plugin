# Project file for IFC2BESTest
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = IFC2BESTest
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += \
	-lIBKMK \
	-lIBK \
	-lTiCPP \
	-lclipper \
	-lIFCPlusPlus \
	-lIFCConvert \
	-lCarve

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/TiCPP/src \
	../../../externals/IFCConvert/src \
	../../../externals/ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
	../../../externals/ifcplusplus/src/IfcPlusPlus/src \
	../../../externals/ifcplusplus/src/external/Carve/src/include \
	../../../externals/ifcplusplus/src/external

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/main.cpp \
	../../src/mainwindow.cpp

HEADERS += \
	../../src/mainwindow.h

FORMS += \
	../../src/mainwindow.ui

