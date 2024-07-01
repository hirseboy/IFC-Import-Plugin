# Project for IFCConvert plugin session and all IBK libraries
# Note: sundials project is excluded because this library used different editor settings
#       Add sundials manually to the session and set build dependencies accordingly!

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += ImportIFCPlugin \
	GEGExportPlugin \
	IFCConvert \
	IFC2BESTest \
	Carve \
	Clipper \
	QtExt \
	IBK \
	IBKMK \
	ifcplusplus \
	Nandrad \
	Vicus \
	CCM \
	glm \
	DataIO \
	TiCPP

# where to find the sub projects
IFC2BESTest.file = ../../IFC2BESTest/projects/Qt/IFC2BESTest.pro

ImportIFCPlugin.file = ../../externals/IFCImportPlugin/projects/Qt/ImportIFCPlugin.pro
GEGExportPlugin.file = ../../externals/GEGExportPlugin/projects/Qt/GEGExportPlugin.pro
IFCConvert.file = ../../externals/IFCConvert/projects/Qt/IFCConvert.pro
CCM.file = ../../externals/CCM/CCM.pro
DataIO.file = ../../externals/DataIO/DataIO.pro
Nandrad.file = ../../externals/Nandrad/Nandrad.pro
Vicus.file = ../../externals/Vicus/Vicus.pro
Carve.file = ../../externals/ifcplusplus/src/IfcPlusPlus/src/external/Carve/projects/Qt/Carve.pro
Clipper.file = ../../externals/clipper/clipper.pro
IBK.file = ../../externals/IBK/IBK.pro
IBKMK.file = ../../externals/IBKMK/IBKMK.pro
TiCPP.file = ../../externals/TiCPP/TiCPP.pro
QtExt.file = ../../externals/QtExt/QtExt.pro
ifcplusplus.file = ../../externals/ifcplusplus/projects/Qt/ifcplusplus.pro
glm.file = ../../externals/glm/glm.pro

# dependencies
IFC2BESTest.depends = ImportIFCPlugin GEGExportPlugin IBK IBKMK TiCPP

CCM.depends = IBK TiCPP
DataIO.depends = IBK
QtExt.depends = IBK
Nandrad.depends = IBK TiCPP IBKMK
Vicus.depends = IBK TiCPP Nandrad IBKMK CCM glm
ImportIFCPlugin.depends = IBK IBKMK TiCPP QtExt Carve Clipper IFCConvert
GEGExportPlugin.depends = IBK IBKMK TiCPP QtExt Nandrad Vicus CCM DataIO
IFCConvert.depends = IBK IBKMK TiCPP Carve Clipper
IBKMK.depends = IBK

linux-g++ | linux-g++-64 | linux-g++-32 {
	# IFC Debug
#    SUBDIRS += IfcDebug
#    IfcDebug.file = ../../IfcDebug/projects/Qt/IfcDebug.pro
 #   IfcDebug.depends = ImportIFCPlugin IBK IBKMK TiCPP
}

#DEFINES += OUTPUT_AFTER_EACH_STEP

