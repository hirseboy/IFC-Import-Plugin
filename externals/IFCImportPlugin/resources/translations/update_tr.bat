@echo off
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set PATH=c:\Qt\5.15.2\msvc2019_64\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
call %VCVARSALL_PATH%

rem 1. Call this batch file to update the ts files.
rem 2. Then edit the ts files with Qt Linguist
rem 3. Then call release_tr.bat to generate release files and copy them to the bin/IBK/resources/translations directory.
lupdate ../../projects/Qt/ImportIFCPlugin.pro

echo Translation files (*.ts) updated, use Qt Linguist to add missing translations!
PAUSE

linguist ImportIFCPlugin_de.ts

