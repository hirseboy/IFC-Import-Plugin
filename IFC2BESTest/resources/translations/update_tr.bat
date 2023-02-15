@echo off
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set PATH=c:\Qt\5.15.2\msvc2019_64\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
call %VCVARSALL_PATH%

rem 1. Call this batch file to update the ts files.
rem 2. Then edit the ts files with Qt Linguist
rem 3. Then call release_tr.bat to generate release files and copy them to the bin/IBK/resources/translations directory.
lupdate ../../../externals/QtExt/projects/Qt/QtExt.pro
lupdate ../../../externals/IFCConvert/projects/Qt/IFCConvert.pro
lupdate ../../../externals/IFCImportPlugin/projects/Qt/ImportIFCPlugin.pro
lupdate ../../projects/Qt/IFC2BESTest.pro

echo Translation files (*.ts) updated, use Qt Linguist to add missing translations!
PAUSE

linguist IFC2BESTest_de.ts ../../../externals/QtExt/resources/translations/QtExt_de.ts ../../../externals/IFCConvert/resources/translations/IFCConvert_de.ts ../../../externals/IFCImportPlugin/resources/translations/ImportIFCPlugin_de.ts

