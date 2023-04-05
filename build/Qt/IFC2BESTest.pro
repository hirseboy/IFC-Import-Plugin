TEMPLATE = subdirs

SUBDIRS = \
        IFC2BESTest \
        IBK \
        IBKMK \
        ifcplusplus \
        IFCConvert \
        TiCPP \
        IFCImportPlugin \
        IfcDebug \
        QtExt \
        clipper \
        Carve



# where to find the sub projects
IFC2BESTest.file = ../../IFC2BESTest/projects/Qt/IFC2BESTest.pro
IfcDebug.file = ../../IfcDebug/projects/Qt/IfcDebug.pro

IFCImportPlugin.file = ../../externals/IFCImportPlugin/projects/Qt/ImportIFCPlugin.pro
IFCConvert.file = ../../externals/IFCConvert/projects/Qt/IFCConvert.pro
clipper.file = ../../externals/clipper/projects/Qt/Clipper.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
ifcplusplus.file = ../../externals/ifcplusplus/projects/Qt/ifcplusplus.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
Carve.file = ../../externals/ifcplusplus/src/IfcPlusPlus/src/external/Carve/projects/Qt/Carve.pro

# dependencies
IFC2BESTest.depends = IBK IBKMK IFCConvert ifcplusplus
IfcDebug.depends = IBK IBKMK IFCImportPlugin IFCConvert ifcplusplus clipper

IBKMK.depends = IBK
TiCPP.depends = IBK
QtExt.depends = IBK
IFCConvert.depends = IBK IBKMK clipper ifcplusplus Carve
IFCImportPlugin.depends = IBK IBKMK IFCConvert ifcplusplus
ifcplusplus.depends = Carve
