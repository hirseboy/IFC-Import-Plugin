# Project file for ImportIFCPlugin
#
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = ImportIFCPlugin

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
	../../src \
	../../../../externals/IBK/src \
	../../../../externals/IBKMK/src \
	../../../../externals/TiCPP/src \
	../../../../externals/QtExt/src \
	../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/IFC4X3/include \
	../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
	../../../ifcplusplus/src/IfcPlusPlus/src \
	../../../ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/include \
	../../../ifcplusplus/src/external \
	../../../ifcplusplus/src/IfcPlusPlus/src/external \
	../../../ifcplusplus/src/IfcPlusPlus/src/external/glm \
	../../../ifcplusplus/src/IfcPlusPlus/src/external/manifold/src/utilities/include \
	../../../ifcplusplus/src/IfcPlusPlus/src/external/Carve/src/common \
	../../../IFCConvert/src

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug$${DIR_PREFIX}
	windows {
				DESTDIR = ../../../lib$${DIR_PREFIX}
				DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
		else {
			DESTDIR = ../../../../bin/debug$${DIR_PREFIX}
		}
}
else {
	OBJECTS_DIR = release
	windows {
				DESTDIR = ../../../lib$${DIR_PREFIX}
				DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}
		else {
			DESTDIR = ../../../../bin/release$${DIR_PREFIX}
		}
}

!windows {
#    QMAKE_POST_LINK += ../../../lib$${DIR_PREFIX}/libImportIFCPlugin.so ../../../../bin/debug$${DIR_PREFIX}/libImportIFCPlugin.so
}

MOC_DIR = moc
UI_DIR = ui

SOURCES += \
	../../src/IFCImportPlugin.cpp \
	../../src/ImportIFCMessageHandler.cpp \
	../../src/ImportWPConvert.cpp \
	../../src/ImportWPRead.cpp \
	../../src/ImportWizard.cpp

HEADERS += \
	../../src/ImportIFCMessageHandler.h \
	../../src/SVCommonPluginInterface.h \
	../../src/SVImportPluginInterface.h \
	../../src/IFCImportPlugin.h \
	../../src/ImportWPConvert.h \
	../../src/ImportWPRead.h \
	../../src/ImportWizard.h

QMAKE_LIBDIR += ../../../lib$${DIR_PREFIX}

LIBS += -L../../../lib$${DIR_PREFIX}

windows {
	LIBS += \
		-lQtExt \
		-lIFCConvert \
		-lclipper \
		-lifcplusplus \
		-lCarve \
		-lTiCPP \
		-lIBKMK \
		-lIBK
}
else {
LIBS += \
	../../../IFCConvert/projects/lib/debug/libIFCConvert.a \
	../../../../SIM-VICUS/externals/lib/debug/libclipper.a \
	../../../ifcplusplus/projects/lib/debug/libifcplusplus.a \
	../../../lib$${DIR_PREFIX}/libCarve.a \
	../../../../SIM-VICUS/externals/lib/debug/libQtExt.a \
	../../../../SIM-VICUS/externals/lib/debug/libTiCPP.a \
	../../../../SIM-VICUS/externals/lib/debug/libIBKMK.a \
	../../../../SIM-VICUS/externals/lib/debug/libIBK.a
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
	../../src/ImportWPConvert.ui \
	../../src/ImportWPRead.ui \
	../../src/ImportWizard.ui

TRANSLATIONS += ../../resources/translations/ImportIFCPlugin_de.ts
CODECFORSRC = UTF-8
