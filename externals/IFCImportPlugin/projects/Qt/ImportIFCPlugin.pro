QT += gui
greaterThan(QT_MAJOR_VERSION, 4):  QT += widgets

TEMPLATE = lib
CONFIG += plugin
CONFIG += shared

CONFIG += c++11

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
	../../../IFCConvert/src \
	../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
	../../../ifcplusplus/src/IfcPlusPlus/src \
	../../../ifcplusplus/src/external/Carve/src/include \
	../../../ifcplusplus/src/external \

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug$${DIR_PREFIX}
	DESTDIR = ../../../lib$${DIR_PREFIX}
	windows {
		DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
}
else {
	OBJECTS_DIR = release
	DESTDIR = ../../../lib$${DIR_PREFIX}
	windows {
		DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}
}

MOC_DIR = moc
UI_DIR = ui

SOURCES += \
	../../src/IFCImportPlugin.cpp \
	../../src/ImportWPConvert.cpp \
	../../src/ImportWPRead.cpp \
	../../src/ImportWizard.cpp

HEADERS += \
	../../src/SVCommonPluginInterface.h \
	../../src/SVImportPluginInterface.h \
	../../src/IFCImportPlugin.h \
	../../src/ImportWPConvert.h \
	../../src/ImportWPRead.h \
	../../src/ImportWizard.h

TRANSLATIONS += \
	ImportIFCPlugin_de_DE.ts

QMAKE_LIBDIR += ../../../../externals/lib$${DIR_PREFIX}

LIBS += -L../../../../externals/lib$${DIR_PREFIX}

LIBS += \
	-lclipper \
	-lIFCPlusPlus \
	-lIFCConvert \
	-lCarve \
	-lTiCPP \
	-lIBKMK \
	-lIBK

win32:LIBS += -liphlpapi
win32:LIBS += -lshell32

# Default rules for deployment.
unix {
	target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
	../../src/ImportWPConvert.ui \
	../../src/ImportWPRead.ui \
	../../src/ImportWizard.ui
