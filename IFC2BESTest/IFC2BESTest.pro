# Project file for IFC2BESTest
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = IFC2BESTest
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../externals/IBK/IBK.pri )

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4) {
	contains(QT_ARCH, i386): {
		DIR_PREFIX =
	} else {
		DIR_PREFIX = _x64
	}
} else {
	DIR_PREFIX =
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(debug, debug|release) {
    DESTDIR = ../bin/debug$${DIR_PREFIX}
}
else {
    DESTDIR = ../bin/release$${DIR_PREFIX}
}

LIBS += -L../lib/debug

LIBS += \
	-lTiCPP \
	-lIBKMK \
	-lIBK

INCLUDEPATH = \
	src \
	../externals/IBK/src \
	../externals/IBKMK/src \
	../externals/TiCPP/src \
	../externals/IFCImportPlugin/src \
	../externals/GEGExportPlugin/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	src/main.cpp \
	src/mainwindow.cpp

HEADERS += \
	src/mainwindow.h

FORMS += \
	src/mainwindow.ui

TRANSLATIONS += resources/translations/IFC2BESTest_de.ts
CODECFORSRC = UTF-8

