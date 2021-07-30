TEMPLATE = subdirs

SUBDIRS = \
		IFC2BESTest \
		IBK \
		IBKMK

# where to find the sub projects
IFC2BESTest.file = ../../IFC2BESTest/projects/Qt/IFC2BESTest.pro

IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro

# dependencies
IFC2BESTest.depends = IBK IBKMK

IBKMK.depends = IBK
