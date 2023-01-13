# -------------------------
# Project for IFC++ library
# -------------------------

# first we define what we are
TARGET = IFCConvert
TEMPLATE = lib

CONFIG += c++11

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../../submodules/SIM-VICUS/externals/IBK/projects/Qt/IBK.pri )

#CONFIG += static
QT       += core gui

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
#

INCLUDEPATH += \
				../../src \
				../../../ifcplusplus/src/IfcPlusPlus/src \
				../../../ifcplusplus/src/IfcPlusPlus/src/ifcpp/reader \
				../../../clipper/src \
				../../../../submodules/SIM-VICUS/externals/IBK/src \
				../../../../submodules/SIM-VICUS/externals/IBKMK/src \
				../../../../submodules/SIM-VICUS/externals/TiCPP/src \
				../../../ifcplusplus/src/external/Carve/src/include \
				../../../ifcplusplus/src/external \
				../../../ifcplusplus/src/external/Carve/src/common

SOURCES +=  \
	../../src/IFCC_Building.cpp \
	../../src/IFCC_BuildingElement.cpp \
	../../src/IFCC_BuildingElementsCollector.cpp \
	../../src/IFCC_BuildingStorey.cpp \
	../../src/IFCC_Clippertools.cpp \
	../../src/IFCC_Component.cpp \
	../../src/IFCC_ComponentInstance.cpp \
	../../src/IFCC_Construction.cpp \
	../../src/IFCC_CurveConverter.cpp \
	../../src/IFCC_Database.cpp \
	../../src/IFCC_EntityBase.cpp \
	../../src/IFCC_FaceConverter.cpp \
	../../src/IFCC_GeometricHelperClasses.cpp \
	../../src/IFCC_Helper.cpp \
	../../src/IFCC_IFCReader.cpp \
	../../src/IFCC_Instances.cpp \
	../../src/IFCC_Material.cpp \
	../../src/IFCC_Opening.cpp \
	../../src/IFCC_ProfileCache.cpp \
	../../src/IFCC_ProfileConverter.cpp \
	../../src/IFCC_Property.cpp \
	../../src/IFCC_Site.cpp \
	../../src/IFCC_Space.cpp \
	../../src/IFCC_SpaceBoundary.cpp \
	../../src/IFCC_SubSurface.cpp \
	../../src/IFCC_SubSurfaceComponent.cpp \
	../../src/IFCC_Surface.cpp \
	../../src/IFCC_GeometryConverter.cpp \
	../../src/IFCC_PlacementConverter.cpp \
	../../src/IFCC_RepresentationConverter.cpp \
	../../src/IFCC_SolidModelConverter.cpp \
	../../src/IFCC_TessellatedItemConverter.cpp \
	../../src/IFCC_Window.cpp \
	../../src/IFCC_WindowGlazing.cpp

HEADERS +=  \
	../../src/IFCC_Building.h \
	../../src/IFCC_BuildingElement.h \
	../../src/IFCC_BuildingElementsCollector.h \
	../../src/IFCC_BuildingStorey.h \
	../../src/IFCC_Clippertools.h \
	../../src/IFCC_Component.h \
	../../src/IFCC_ComponentInstance.h \
	../../src/IFCC_Construction.h \
	../../src/IFCC_CurveConverter.h \
	../../src/IFCC_Database.h \
	../../src/IFCC_EntityBase.h \
	../../src/IFCC_FaceConverter.h \
	../../src/IFCC_GeometricHelperClasses.h \
	../../src/IFCC_Helper.h \
	../../src/IFCC_IFCReader.h \
	../../src/IFCC_Instances.h \
	../../src/IFCC_Logger.h \
	../../src/IFCC_Material.h \
	../../src/IFCC_Opening.h \
	../../src/IFCC_ProfileCache.h \
	../../src/IFCC_ProfileConverter.h \
	../../src/IFCC_Property.h \
	../../src/IFCC_Site.h \
	../../src/IFCC_Space.h \
	../../src/IFCC_SpaceBoundary.h \
	../../src/IFCC_SubSurface.h \
	../../src/IFCC_SubSurfaceComponent.h \
	../../src/IFCC_Surface.h \
	../../src/IFCC_TessellatedItemConverter.h \
	../../src/IFCC_Types.h \
	../../src/IFCC_GeometryConverter.h \
	../../src/IFCC_PlacementConverter.h \
	../../src/IFCC_RepresentationConverter.h \
	../../src/IFCC_SolidModelConverter.h \
	../../src/IFCC_Window.h \
	../../src/IFCC_WindowGlazing.h

unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}
