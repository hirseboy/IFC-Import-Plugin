@echo off

:: Check if VC environment is already set (e.g. from CI or Developer Command Prompt)
where cl >nul 2>nul
if %ERRORLEVEL% equ 0 (
	echo VC environment already active, skipping vcvarsall.bat
	goto :vc_ready
)

:: setup VC 2022 environment variables
:: Use vswhere to find VS 2022 installation automatically
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -version [17.0^,18.0^) -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSINSTALLDIR=%%i

if not defined VSINSTALLDIR (
	echo ** Could not find Visual Studio 2022 installation **
	exit /b 1
)
echo Using Visual Studio at: %VSINSTALLDIR%
call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64

:vc_ready

:: For different Qt installations, please set the environment variables JOM_PATH and CMAKE_PREFIX_PATH
:: for the current Windows user. Also, make sure cmake is in the PATH variable.
:: Mind: the dlls in the release/win/VC14_xxx subdirectories must match the Qt version for building.
::       You must copy the Qt dlls used for building into these directories.
::
:: For debugging crashes on Windows, change the CMAKE_BUILD_TYPE to "RelWithDebInfo".

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Tools\QtCreator\bin\jom
)
:: Qt6_DIR (set by CI) takes priority over CMAKE_PREFIX_PATH
if defined Qt6_DIR (
	for %%d in ("%Qt6_DIR%\..\..\..") do set CMAKE_PREFIX_PATH=%%~fd
) else if not defined CMAKE_PREFIX_PATH (
	set CMAKE_PREFIX_PATH=c:\Qt\6.9.3\msvc2022_64
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

:: create and change into build subdir
mkdir bb_VC_x64
pushd bb_VC_x64

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" .. -DCMAKE_BUILD_TYPE:String="Release"
jom
if ERRORLEVEL 1 GOTO fail

popd

:: copy executable to bin/release dir
xcopy /Y .\bb_VC_x64\IFC2BESTest\IFC2BESTest.exe ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\ifcplusplus\ifcplusplus.dll ..\..\bin\release_x64

exit /b 0


:fail
echo ** Build Failed **
exit /b 1
