# Project file for IFC2BESTest
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = IFC2BESTest
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT -= core gui

CONFIG += console
CONFIG -= app_bundle

#DEFINES += IFCQUERY_STATIC_LIB

LIBS += \
	-lIBKMK \
	-lIBK \
	-lIFCPlusPlus

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
	../../../externals/ifcplusplus/src/IfcPlusPlus/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	../../src/main.cpp
