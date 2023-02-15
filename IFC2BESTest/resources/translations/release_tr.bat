@echo off
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set PATH=c:\Qt\5.15.2\msvc2019_64\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
call %VCVARSALL_PATH%

lrelease ../../../externals/QtExt/resources/translations/QtExt_de.ts ../../../externals/IFCConvert/resources/translations/IFCConvert_de.ts ../../../externals/IFCImportPlugin/resources/translations/ImportIFCPlugin_de.ts -qm ImportIFCPlugin_de.qm
lrelease ../../../externals/QtExt/resources/translations/QtExt_de.ts ../../../externals/IFCConvert/resources/translations/IFCConvert_de.ts ../../../externals/IFCImportPlugin/resources/translations/ImportIFCPlugin_de.ts IFC2BESTest_de.ts -qm IFC2BESTest_de.qm
:: lrelease ../../../externals/QtExt/resources/translations/QtExt_es.ts SIM-VICUS_es.ts -qm SIM-VICUS_es.qm

pause
