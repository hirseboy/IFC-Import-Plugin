# Project file for GEGExportPlugin
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = GEGExportPlugin

QT += gui
greaterThan(QT_MAJOR_VERSION, 4):  QT += widgets

TEMPLATE = lib
CONFIG += plugin
CONFIG += shared

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# check if 32 or 64 bit version and set prefix variable for using in output paths
contains(QT_ARCH, i386): {
	DIR_PREFIX =
} else {
	DIR_PREFIX = _x64
}

INCLUDEPATH = \
	src \
	../IBK/src \
	../IBKMK/src \
	../TiCPP/src \
	../Nandrad/src \
	../Vicus/src \
	../glm/src \
	../glm/src\glm \
	../glm/src\gtx \
	../QtExt/src

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug$${DIR_PREFIX}
	windows {
		DESTDIR = ../lib$${DIR_PREFIX}
		DLLDESTDIR = ../../bin/debug$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../bin/debug$${DIR_PREFIX}
	}
}
else {
	OBJECTS_DIR = release
	windows {
		DESTDIR = ../lib$${DIR_PREFIX}
		DLLDESTDIR =../../bin/release$${DIR_PREFIX}
	}
	else {
		DESTDIR = ../../bin/release$${DIR_PREFIX}
	}
}

!windows {
#    QMAKE_POST_LINK += ../lib$${DIR_PREFIX}/libGEGExportPlugin.so ../../bin/debug$${DIR_PREFIX}/libGEGExportPlugin.so
}

MOC_DIR = moc
UI_DIR = ui

SOURCES += \
	src/ExportWPRead.cpp \
	src/GEGConstruction.cpp \
	src/GEGExportPlugin.cpp \
	src/GEGExportMessageHandler.cpp \
	src/ExportWizard.cpp \
	src/GEGMaterial.cpp \
	src/GEGRoom.cpp \
	src/GEGSurface.cpp \
	src/GEGZone.cpp \
	src/GEG_Utilities.cpp

HEADERS += \
	src/ExportWPRead.h \
	src/GEGConstruction.h \
	src/GEGExportMessageHandler.h \
	src/GEGMaterial.h \
	src/GEGRoom.h \
	src/GEGSurface.h \
	src/GEGZone.h \
	src/GEG_Utilities.h \
	src/SVCommonPluginInterface.h \
	src/SVExportPluginInterface.h \
	src/GEGExportPlugin.h \
	src/ExportWizard.h


CONFIG(debug, debug|release) {
	QMAKE_LIBDIR += ../lib$${DIR_PREFIX} -L../lib/debug
	LIBS += -L../lib$${DIR_PREFIX} -L../lib/debug
}
else {
	QMAKE_LIBDIR += ../lib$${DIR_PREFIX} -L../lib/release
	LIBS += -L../lib$${DIR_PREFIX} -L../lib/release
}

windows {
	LIBS += \
		-lQtExt \
		-lDataIO \
		-lCCM \
		-lVicus \
		-lNandrad \
		-lTiCPP \
		-lIBKMK \
		-lIBK
}
else {
    CONFIG(debug, debug|release) {
	LIBS += \
	    ../lib/debug/libQtExt.a \
	    ../lib/debug/libDataIO.a \
	    ../lib/debug/libCCM.a \
	    ../lib/debug/libVicus.a \
	    ../lib/debug/libNandrad.a \
	    ../lib/debug/libTiCPP.a \
	    ../lib/debug/libIBKMK.a \
	    ../lib/debug/libIBK.a
    }
    else {
	LIBS += \
	    ../lib/release/libQtExt.a \
	    ../lib/release/libDataIO.a \
	    ../lib/release/libCCM.a \
	    ../lib/release/libVicus.a \
	    ../lib/release/libNandrad.a \
	    ../lib/release/libTiCPP.a \
	    ../lib/release/libIBKMK.a \
	    ../lib/release/libIBK.a
    }
}

win32:LIBS += -liphlpapi
win32:LIBS += -lshell32

win32-msvc* {
	QMAKE_CXXFLAGS += /std:c++17
}


# Default rules for deployment.
# unix {
# 	target.path = $$[QT_INSTALL_PLUGINS]/generic
# }
# !isEmpty(target.path): INSTALLS += target

FORMS += \
	src/ExportWPRead.ui \
	src/ExportWizard.ui

TRANSLATIONS += ../../resources/translations/GEGExportPlugin_de.ts
CODECFORSRC = UTF-8
