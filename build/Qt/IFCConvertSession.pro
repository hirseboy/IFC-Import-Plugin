# Project for IFCConvert plugin session and all IBK libraries
# Note: sundials project is excluded because this library used different editor settings
#       Add sundials manually to the session and set build dependencies accordingly!

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += ImportIFCPlugin \
	IFCConvert \
	IFC2BESTest \
	Carve \
	Clipper \
	QtExt \
	IBK \
	IBKMK \
	TiCPP

# where to find the sub projects
IFC2BESTest.file = ../../IFC2BESTest/projects/Qt/IFC2BESTest.pro

ImportIFCPlugin.file = ../../externals/IFCImportPlugin/projects/Qt/ImportIFCPlugin.pro
IFCConvert.file = ../../externals/IFCConvert/projects/Qt/IFCConvert.pro
Carve.file = ../../externals/ifcplusplus/src/external/Carve/projects/Qt/Carve.pro
Clipper.file = ../../externals/Clipper/projects/Qt/Clipper.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro

# dependencies
IFC2BESTest.depends = ImportIFCPlugin IBK IBKMK TiCPP

QtExt.depends = IBK
ImportIFCPlugin.depends = IBK IBKMK TiCPP QtExt Carve Clipper IFCConvert
IFCConvert.depends = IBK IBKMK TiCPP Carve Clipper
IBKMK.depends = IBK

#DEFINES += OUTPUT_AFTER_EACH_STEP

